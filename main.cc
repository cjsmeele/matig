#include <cstdio>
#include <iostream>
#include <vector>
#include <stdexcept>
#include <string>
#include <sstream>

struct Token {
    enum class Type {
        NONE,
        LIST_START,
        LIST_END,
        ATOM_STRING,
        ATOM_NUMERIC,
        ATOM_SYMBOL,
        QUOTE,
    } type;

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

std::vector<Token> tokenize(const std::string &text) {
    std::vector<Token> tokens;

    try {
        auto isBreak = [](char c) -> bool {
            return (c < ' '
                    || c == ' '
                    || c == '('
                    || c == ')');
        };
        auto isBreakAt = [&text, &isBreak](size_t i) -> bool {
            if (i >= text.length())
                return true;
            return isBreak(text[i]);
        };

        size_t i = 0;
        char c;

        auto next = [&text, &i, &c]() {
            i++;
            if (i < text.length())
                c = text[i];
            else
                c = '\0';
        };

        while (i < text.length()) {
            Token token;

            // Parse a token and leave i to point at the start of the
            // next token.

            c = text[i];

            if (c == ';') {
                for (; i < text.length() && c != '\n'; next());
                continue;
            } else if (c >= '0' && c <= '9') {
                token.type = Token::Type::ATOM_NUMERIC;

                for (; !isBreak(c); next()) {
                    if (c < '0' || c > '9')
                        throw std::runtime_error("Parse error: Invalid numeric");
                    token.content += c;
                }
            } else if (c == '"') {
                token.type = Token::Type::ATOM_STRING;
                c = text.at(++i);

                bool inEscape = false;
                for (; c; next()) {
                    if (inEscape) {
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
                        inEscape = false;
                    } else {
                        if (c == '\\') {
                            inEscape = true;
                        } else {
                            if (c == '"') {
                                i++;
                                break;
                            }
                            token.content += c;
                        }
                    }
                }
            } else if (c == '(') {
                token.type = Token::Type::LIST_START;
                token.content += c;
                i++;
            } else if (c == ')') {
                token.type = Token::Type::LIST_END;
                token.content += c;
                i++;
            } else if (isBreak(c)) {
                i++;
                continue;
            } else {
                token.type = Token::Type::ATOM_SYMBOL;

                for (; !isBreak(c); next()) {
                    token.content += c;
                }
            }
            
            tokens.emplace_back(token);
        }
    } catch (std::out_of_range &e) {
        throw;
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

std::vector<Node> read(const std::string &text) {
    auto tokens = tokenize(text);
    std::vector<Node> nodes;
    std::stack <Node> context;

    Node currentList = { };

    std::vector<Node> *container = &nodes;

    for (auto t : tokens) {
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
                throw std::runtime_error("Unmatched closing paren");

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
    return nodes;
}

Node eval(const Node &node) {
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
        throw std::runtime_error("Invalid node type");
    }
}

void print(const Node &node, int depth = 0) {

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

int main(int argc, char **argv) {

    std::string input;

    while (true) {
        int c = fgetc(stdin);
        if (c <= 0)
            break;

        input += c;
    }

    input += '\n';

    auto nodes = read(input);
    for (const auto &node : nodes) {
        print(node);
    }

    return 0;
}
