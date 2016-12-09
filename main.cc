#include <cstdio>
#include <iostream>
#include <vector>
#include <stdexcept>
#include <string>
#include <sstream>

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

struct Node {
    enum class Type {
        NONE = 0,
        LIST,
        ATOM,
    } type;

    enum class AtomType {
        NONE = 0,
        STRING,
        INTEGER,
        FLOAT,
        SYMBOL,
    } atomType;

    std::vector<Node> children;
    std::string valueSymbol;
    std::string valueString;
    int valueInt;
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

#include <unordered_map>

class Environment {
    std::unordered_map<std::string, Node> symbols;
public:
    Environment() = default;
};


#include <stack>

/**
 * \brief Read one textual expression into an AST node.
 */
static Node read(std::istream &stream) {

    std::vector<Token> tokens = tokenize(stream);

    std::vector<Node> nodes;
    std::stack <Node> context;

    Node currentList = { };

    std::vector<Node> *container = &nodes;

    for (const auto &t : tokens) {
        std::cout << (int)t.type << ": " << t.content << "\n";
        if (t.type == Token::Type::LIST_START) {
            if (currentList.type != Node::Type::NONE) {
                context.emplace(currentList);
            }
            currentList = { };
            currentList.type = Node::Type::LIST;
            container = &currentList.children;

        } else if (t.type == Token::Type::LIST_END) {
            if (currentList.type == Node::Type::NONE)
                throw SyntaxError("Unmatched closing paren");

            if (context.empty()) {
                nodes.emplace_back(currentList);
                currentList = { };
                container = &nodes;
            } else {
                Node parent = context.top();
                parent.children.emplace_back(currentList);
                currentList = parent;
                context.pop();
                container = &currentList.children;
            }
        } else if (t.type == Token::Type::ATOM_NUMERIC) {
            Node node;
            node.type     = Node::Type::ATOM;
            node.atomType = Node::AtomType::INTEGER;
            node.valueInt = std::stoi(t.content);
            container->emplace_back(node);
        } else if (t.type == Token::Type::ATOM_STRING) {
            Node node;
            node.type        = Node::Type::ATOM;
            node.atomType    = Node::AtomType::STRING;
            node.valueString = t.content;
            container->emplace_back(node);
        } else if (t.type == Token::Type::ATOM_SYMBOL) {
            Node node;
            node.type        = Node::Type::ATOM;
            node.atomType    = Node::AtomType::SYMBOL;
            node.valueSymbol = t.content;
            container->emplace_back(node);
        }
    }

    if (nodes.size() == 1)
        return nodes[0];
    else if (nodes.size() == 0)
        return Node();
    else
        throw std::logic_error("Read more than one node");
}

static Node eval(const Node &node) {
    if (node.type == Node::Type::LIST) {
        return Node { Node::Type::ATOM,
                Node::AtomType::STRING,
                { }, "", "ListResult", 0 };
    } else if (node.type == Node::Type::ATOM) {
        if (node.atomType == Node::AtomType::SYMBOL) {
            return Node { Node::Type::ATOM,
                    Node::AtomType::STRING,
                    { }, "", "SymbolResult", 0 };
        } else {
            return node;
        }
    } else {
        throw std::logic_error("Invalid node type");
    }
}

static void print(const Node &node, int depth = 0) {

    std::string indent;
    indent.resize(depth, ' ');

    if (node.type == Node::Type::LIST) {
        std::cout << indent << "(\n";
        for (const auto &child : node.children)
            print(child, depth + 1);
        std::cout << indent << ")";
    } else {
        if (node.atomType == Node::AtomType::INTEGER)
            std::cout << indent << node.valueInt;
        else if (node.atomType == Node::AtomType::STRING)
            std::cout << indent << '"' << node.valueString << '"';
        else if (node.atomType == Node::AtomType::SYMBOL)
            std::cout << indent << node.valueSymbol;
        // std::cout << " ";
    }
    std::cout << "\n";
}

#include <unistd.h>

int main(int argc, char **argv) {

    while (true) {
        if (isatty(fileno(stdin))) {
            std::cout << "MATIG> ";
            std::cout.flush();
        }
        auto node = read(std::cin);
        if (node.type == Node::Type::NONE)
            break;

        // print(eval(node));
        print(node);
    }

    return 0;
}
