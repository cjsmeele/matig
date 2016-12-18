/**
 * \file
 * \brief
 * \author    Chris Smeele
 * \copyright Copyright (c) 2016, Chris Smeele
 * \license   MIT, see LICENSE.
 */
#include "read.hh"

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
        return (iscntrl(c)
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

            if (c == '\'') {
                token.type = Token::Type::QUOTE;
                token.content = c;

                c = next(false);

            } else if (c == '(') {
                token.type = Token::Type::LIST_START;
                token.content = c;

                listLevel++;

                c = next(false);

            } else if (c == '.') {
                token.type = Token::Type::CONS_DOT;
                token.content = c;

                c = next(false);
                if (!isBreak(c))
                    throw SyntaxError("Invalid cons dot");

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

            } else if (!iscntrl(c)){
                token.type    = Token::Type::ATOM_SYMBOL;
                token.content = c;

                while (!isBreak((c = next(false))))
                    token.content += c;

            } else {
                throw SyntaxError("Unexpected text: char "s + std::to_string(c));
            }

            // End of token.
            tokens.push_back(token);

            if (listLevel == 0 && (token.type == Token::Type::LIST_END
                                   || token.isAtomish())) {
                // We have a complete expression (a complete list or one atom).
                // We haven't reached the end of the stream yet.
                stream.putback(c);
                break;
            }

        } catch (EndOfInput &e) {
            // We've reached the end of the stream at the end of a
            // token, while at the top listlevel.

            if (token.type == Token::Type::NONE)
                throw LogicError("Got empty token");

            tokens.push_back(token);
            break;
        }
    }

    return tokens;
}

/**
 * \brief Create an atom from a token.
 */
static std::shared_ptr<AtomExpr> readAtom(const Token &token) {
    if (token.type == Token::Type::ATOM_NUMERIC) {
        return std::make_shared<NumericExpr>(std::stoll(token.content));

    } else if (token.type == Token::Type::ATOM_STRING) {
        return std::make_shared<StringExpr>(token.content);

    } else if (token.type == Token::Type::ATOM_SYMBOL) {
        return std::make_shared<SymbolExpr>(token.content);

    } else {
        throw LogicError("Not an atom token: '"s + token.content + "'");
    }
}

/**
 * \brief Create a nested cons from a series of tokens.
 */
template<typename It>
static std::shared_ptr<Expr> readCons(const It &start, const It &end) {

    if (start->type != Token::Type::LIST_START)
        throw LogicError("Tokens do not specify a cons (start))");
    if ((end-1)->type != Token::Type::LIST_END)
        throw LogicError("Tokens do not specify a cons (end)");

    auto size = std::distance(start, end);

    if (size > 2) {
        auto rootCons         = std::make_shared<ConsExpr>();
        ConsExpr *currentCons = rootCons.get();

        bool haveDot = false; // Whether the previous token was a dot.
        int quotes = 0;

        // Loop through all tokens within the parentheses.
        for (auto it = start + 1; it != end - 1; it++) {

            Eptr currentExpr = nullptr;

            if (it->type == Token::Type::CONS_DOT) {
                if (haveDot)
                    throw SyntaxError("Invalid dot syntax (second dot)");
                if (!currentCons->getCar())
                    throw SyntaxError("Invalid dot syntax (no car)");
                if (currentCons->getCdr())
                    throw SyntaxError("Invalid dot syntax");
                if (quotes)
                    throw SyntaxError("Stray quote before cons dot");

                haveDot = true;

            } else if (it->isAtomish()) {

                currentExpr = readAtom(*it);

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
                    throw LogicError("Bad list token nesting");

                currentExpr = readCons(subListStart, it+1);

            } else if (it->type == Token::Type::QUOTE) {
                quotes++;

            } else {
                throw LogicError("Bad token type");
            }

            // For reference: Lisp syntax => { car cdr }
            // (1)       => { 1 nil }
            // (1 . 2)   => { 1 2 }
            // (1 2 . 3) => { 1 { 2 3 } }
            // (1 2 3)   => { 1 { 2 { 3 nil } } }
            if (currentExpr) {
                // Translate quote syntax to (quote ...).
                currentExpr = currentExpr->quote(quotes);
                quotes = 0;

                if (haveDot) {
                    currentCons->getCdr() = currentExpr;
                    haveDot = false;
                } else {
                    if (currentCons->getCar()) {
                        // Make next cons.
                        currentCons->getCdr() = std::make_shared<ConsExpr>();
                        currentCons = static_cast<ConsExpr*>(currentCons->getCdr().get());
                    }
                    currentCons->getCar() = currentExpr;
                }
            }
        }
        if (haveDot)
            throw SyntaxError("Invalid dot syntax");
        if (quotes)
            throw SyntaxError("Invalid quote syntax");

        if (!currentCons->getCar())
            currentCons->getCdr() = std::make_shared<SymbolExpr>("nil");

        if (!currentCons->getCdr())
            currentCons->getCdr() = std::make_shared<SymbolExpr>("nil");

        return rootCons;

    } else {
        // Empty list.
        return std::make_shared<SymbolExpr>("nil");
    }
}

/**
 * \brief Read an expression from a list of tokens.
 */
static Eptr read(const std::vector<Token> &tokens) {

    if (!tokens.size())
        return nullptr;

    int quotes = 0;
    size_t i = 0;
    for (; i < tokens.size() && tokens[i].type == Token::Type::QUOTE; i++)
        quotes++;

    if (quotes && i >= tokens.size())
        throw SyntaxError("Quote at EOF");

    if (   tokens[i].type == Token::Type::ATOM_NUMERIC
        || tokens[i].type == Token::Type::ATOM_STRING
        || tokens[i].type == Token::Type::ATOM_SYMBOL) {

        return readAtom(tokens[i])->quote(quotes);

    } else if (tokens[i].type == Token::Type::LIST_START) {
        
        return readCons(tokens.begin()+i, tokens.end())->quote(quotes);

    } else {
        throw LogicError("Unsupported token type");
    }
}

Eptr read(std::istream &stream) {
    return read(tokenize(stream));
}
