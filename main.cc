#include <cstdio>
#include <iostream>
#include <vector>
#include <stdexcept>
#include <string>
#include <sstream>
#include <fstream>
#include <memory>

#include "expression.hh"
#include "environment.hh"
#include "function.hh"

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

    std::istream  *in = &std::cin;
    std::ifstream file;

    if (argc > 1 && std::string(argv[1]) != "-") {
        file.open(argv[1]);
        if (!file)
            throw std::runtime_error(std::string("Could not open file '")
                                     + argv[1] + "' for reading.");
    }

    if (file.is_open())
        in = &file;

    bool isInteractive = in == &std::cin && isatty(fileno(stdin));

    while (true) {
        if (isInteractive) {
            std::cout << "MATIG> ";
            std::cout.flush();
        }
        try {
            auto expr = read(*in);
            if (!expr)
                break;

            auto result = eval(*expr);
            if (isInteractive)
                print(*result);

        } catch (std::runtime_error &e) {
            std::cerr << "ERR: " << e.what() << "\n";
        }
    }

    return 0;
}
