#pragma once

#include "common.hh"

#include <vector>
#include <istream>

#include "expression.hh"

/**
 * \brief An error in a program's syntax (mismatched parentheses, bad
 *        literals, etc.).
 */
class SyntaxError : public ProgramError {
public:
    SyntaxError(const std::string &s)
        : ProgramError("Syntax error: " + s) { }
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
 * \brief Read one textual expression into an s-expression.
 */
Eptr read(std::istream &stream);
