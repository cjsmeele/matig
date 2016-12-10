#include <cstdio>
#include <iostream>
#include <vector>
#include <stdexcept>
#include <string>
#include <sstream>
#include <map>
#include <memory>

class SyntaxError : public std::runtime_error {
public:
    SyntaxError(const std::string &s)
        : std::runtime_error("Syntax error: " + s) { }
};

struct Token {
    enum class Type {
        NONE,
        LIST_START,
        LIST_END,
        ATOM_STRING,
        ATOM_NUMERIC,
        ATOM_SYMBOL,
        QUOTE,
        } type = Type::NONE;

    std::string content;
};

class Expression;
class Function;

struct Symbol {
    // std::string asSymbol;
    // std::string asString;
    // int64_t     asInt;
    Expression  *asExpression;
    Function    *asFunction;
};

class Environment {
    std::map<std::string, Symbol> symbols;
    Environment *parent;
public:
    Symbol &lookup(const std::string &name) {
        auto it = symbols.find(name);
        if (it == symbols.end()) {
            if (parent)
                return parent->lookup(name);
            else 
                throw std::runtime_error("Symbol does not exist");
        } else {
            return it->second;
        }
    }

    void set(const std::string &name, const Symbol &value) {
        symbols[name] = value;
    }

    // Environment(Environment *parent = nullptr)
    //     : parent(parent) {
    // }
    Environment(Environment *parent = nullptr);
};

class Expression;
typedef std::unique_ptr<Expression> Eptr;

class Expression {
public:
    virtual std::string repr() const = 0;
    virtual Eptr eval(Environment &env) const = 0;
    virtual ~Expression() = default;
};

class AtomExpression : public Expression { };

class NumericExpression : public AtomExpression {

    int64_t value;

public:
    std::string repr() const override {
        return std::to_string(value);
    }
    Eptr eval(Environment &env) const override {
        return std::make_unique<NumericExpression>(value);
    }

    NumericExpression(int64_t value)
        : value(value) { }
};

class StringExpression : public AtomExpression {

    std::string value;

public:
    std::string repr() const override {
        // TODO: Escaping.
        return std::string("\"") + value + '"';
    }
    Eptr eval(Environment &env) const override {
        return std::make_unique<StringExpression>(value);
    }

    StringExpression(const std::string &value)
        : value(value) { }
};

class SymbolExpression : public AtomExpression {

    std::string value;

public:
    std::string repr() const override {
        return value;
    }
    Eptr eval(Environment &env) const override {
        return std::make_unique<SymbolExpression>(value);
    }

    SymbolExpression(const std::string &value)
        : value(value) { }
};

class Function {
public:
    virtual Eptr operator()(std::vector<Eptr> parameters,
                       Environment &env) = 0;

    virtual ~Function() = default;
};

class FunctionBuiltin : public Function {
    typedef std::function<Eptr (std::vector<Eptr>, Environment&)> F;

    F func;

public:
    Eptr operator()(std::vector<Eptr> parameters,
                    Environment &env) {

        return func(std::move(parameters), env);
    }

    FunctionBuiltin(F func)
        : func(func) { }
};

class FunctionLisp : public Function {

public:
    Eptr operator()(std::vector<Eptr> parameters,
                    Environment &env) {
        return std::make_unique<NumericExpression>(42);
    }
};

class ListExpression : public Expression {

    std::vector<Eptr> children;

public:
    std::string repr() const override {
        if (children.size()) {
            // return "<list>";
            std::string s = "(";
            for (const auto &child : children)
                s += child->repr() + " ";
            if (s[s.length()-1] == ' ')
                s.erase(s.end()-1);
            s += ")";
            return s;
        } else {
            return "nil";
        }
    }
    Eptr eval(Environment &env) const override {
        // return std::make_unique<ListExpression>(children);
        // return std::make_unique<NumericExpression>(55);
        if (children.size()) {
            auto &first = children[0];
            auto symExpr = dynamic_cast<SymbolExpression*>(first.get());

            if (!symExpr)
                throw std::runtime_error("First list element is not a symbol");

            auto sym = env.lookup(symExpr->repr());

            if (!sym.asFunction)
                throw std::runtime_error("Symbol's function slot is empty");
            
            std::vector<Eptr> parameters;

            for (auto it = children.begin() + 1; it != children.end(); it++)
                parameters.push_back((*it)->eval(env));

            return (*sym.asFunction)(std::move(parameters), env);

        } else {
            return std::make_unique<ListExpression>();
        }
    }

    ListExpression()
        : children{ } { }

    ListExpression(std::vector<Eptr> children)
        : children(std::move(children)) { }
};

Environment::Environment(Environment *parent)
    : parent(parent) {

    // XXX

    static FunctionBuiltin f([](std::vector<Eptr> parameters, Environment &env) {
            std::cout << parameters.at(0)->repr() << "\n";
            return std::move(parameters.at(0));
        });
    
    set("print", { nullptr, &f });
}

/**
 * \brief Tokenize one textual expression.
 */
static std::vector<Token> tokenize(std::istream &stream) {

    std::vector<Token> tokens;
    int listLevel = 0;

    class EndOfInput { };

    auto next = [&](bool required) {
        char c;
        if (stream.get(c)) {
            return c;
        } else if (listLevel) {
            throw SyntaxError("Unexpected EOF while reading list expression");
        } else if (required) {
            throw SyntaxError("Unexpected EOF while reading expression");
        } else {
            throw EndOfInput();
        }
    };

    auto isBreak = [](char c) -> bool {
        return (c < ' '
                || isspace(c)
                || c == '('
                || c == ')');
    };

    char c = '\0';

    while (true) {
        try {
            if (!c)
                c = next(false);

            // Skip whitespace.
            if (isspace(c)) {
                c = '\0';
                continue;
            }

            // Skip comments.
            if (c == ';') {
                while (next(false) != '\n');
                c = '\0';
                continue;
            }
        } catch (EndOfInput &e) {
            break;
        }

        Token token;

        try {
            // Parse one token.
            // At the end of this if/elseif chain, exactly one
            // character has been read beyond the end of the token.

            if (c == '(') {
                token.type = Token::Type::LIST_START;
                token.content = c;

                listLevel++;

                c = next(false);

            } else if (c == ')') {
                token.type = Token::Type::LIST_END;
                token.content = c;

                listLevel--;
                if (listLevel < 0)
                    throw SyntaxError("Unexpected end of list while reading atom expression");

                c = next(false);

            } else if (isdigit(c)) {
                token.type = Token::Type::ATOM_NUMERIC;
                token.content = c;

                while (isdigit(c = next(false)))
                    token.content += c;

                if (!isBreak(c))
                    throw SyntaxError("Invalid numeric");

            } else if (c == '"') {
                token.type = Token::Type::ATOM_STRING;

                while (true) {
                    c = next(true);

                    if (c == '\\') {
                        c = next(true);
                        if (c == 'n')
                            token.content += '\n';
                        else if (c == 'r')
                            token.content += '\r';
                        else if (c == 't')
                            token.content += '\t';
                        else if (c == 'v')
                            token.content += '\v';
                        else
                            token.content += c;
                    } else {
                        if (c == '"')
                            break;
                        else
                            token.content += c;
                    }
                }

                c = next(false);
                if (!isBreak(c))
                    throw SyntaxError("Invalid string");

            } else if (c > ' '){
                token.type    = Token::Type::ATOM_SYMBOL;
                token.content = c;

                while (!isBreak((c = next(false))))
                    token.content += c;

            } else {
                throw SyntaxError("Unexpected text");
            }

            // End of token.
            tokens.push_back(token);

            if (listLevel == 0) {
                // We have a complete expression (a complete list or one atom).
                // We haven't reached the end of the stream yet.
                stream.putback(c);
                break;
            }

        } catch (EndOfInput &e) {
            // We've reached the end of the stream at the end of a
            // token, while at the top listlevel.

            if (token.type == Token::Type::NONE)
                throw std::logic_error("Got empty token");

            tokens.push_back(token);
            break;
        }
    }

    return tokens;
}

#include <stack>

/**
 * \brief Create an atom from a token.
 */
static std::unique_ptr<AtomExpression> readAtom(const Token &token) {
    if (token.type == Token::Type::ATOM_NUMERIC) {
        return std::make_unique<NumericExpression>(std::stoll(token.content));

    } else if (token.type == Token::Type::ATOM_STRING) {
        return std::make_unique<StringExpression>(token.content);

    } else if (token.type == Token::Type::ATOM_SYMBOL) {
        return std::make_unique<SymbolExpression>(token.content);

    } else {
        throw std::logic_error("Not an atom token");
    }
}

/**
 * \brief Create a list from a series of tokens.
 */
template<typename It>
static std::unique_ptr<ListExpression> readList(const It &start, const It &end) {
    if (start->type != Token::Type::LIST_START)
        throw std::logic_error("Tokens do not specify a list (start))");
    if ((end-1)->type != Token::Type::LIST_END)
        throw std::logic_error("Tokens do not specify a list (end)");

    auto size = std::distance(start, end);

    if (size > 2) {
        std::vector<Eptr> children;

        for (auto it = start + 1; it != end - 1; it++) {
            if (it->type == Token::Type::ATOM_NUMERIC
                || it->type == Token::Type::ATOM_STRING
                || it->type == Token::Type::ATOM_SYMBOL) {

                children.push_back(readAtom(*it));

            } else if (it->type == Token::Type::LIST_START) {
                auto subListStart = it;
                int depth = 1;
                do {
                    it++;
                    if (it == end - 1)
                        break;
                    if (it->type == Token::Type::LIST_START)
                        depth++;
                    if (it->type == Token::Type::LIST_END)
                        depth--;
                } while (depth);

                if (depth)
                    throw std::logic_error("Bad list token nesting");

                children.push_back(readList(subListStart, it+1));

            } else {
                throw std::logic_error("Bad token type");
            }
        }
        return std::make_unique<ListExpression>(std::move(children));

    } else {
        return std::make_unique<ListExpression>();
    }
}

/**
 * \brief Read an expression from a list of tokens.
 */
static Eptr read(const std::vector<Token> &tokens) {

    if (!tokens.size())
        return nullptr;

    if (tokens[0].type == Token::Type::ATOM_NUMERIC
        || tokens[0].type == Token::Type::ATOM_STRING
        || tokens[0].type == Token::Type::ATOM_SYMBOL) {

        return readAtom(tokens[0]);

    } else if (tokens[0].type == Token::Type::LIST_START) {
        
        return readList(tokens.begin(), tokens.end());

    } else {
        throw std::logic_error("Unsupported token");
    }
}

/**
 * \brief Read one textual expression into an s-expression.
 */
static Eptr read(std::istream &stream) {
    return read(tokenize(stream));
}

static Eptr eval(const Expression &expr) {
    Environment env;
    return expr.eval(env);
}

static void print(const Expression &expr, int depth = 0) {
    std::string indent;
    indent.resize(depth, ' ');

    std::cout << expr.repr() << "\n";
}

#include <unistd.h>

int main(int argc, char **argv) {

    while (true) {
        if (isatty(fileno(stdin))) {
            std::cout << "MATIG> ";
            std::cout.flush();
        }
        auto expr = read(std::cin);
        if (!expr)
            break;

        print(*eval(*expr));
    }

    return 0;
}
