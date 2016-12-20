/**
 * \file
 * \brief     Read function.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2016, Chris Smeele
 * \license   MIT, see LICENSE.
 */
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

/**
 * \brief Token class.
 */
struct Token {
    enum class Type {
        NONE,
        LIST_START,
        CONS_DOT,
        LIST_END,
        ATOM_STRING,
        ATOM_NUMERIC,
        ATOM_SYMBOL,
        QUOTE,
    } type = Type::NONE;

    std::string content;

    bool isAtomish() const {
        return type == Type::ATOM_STRING
            || type == Type::ATOM_NUMERIC
            || type == Type::ATOM_SYMBOL;
    }
};

/**
 * \brief Read one textual expression into an s-expression.
 *
 * \param stream The stream to read an expression from
 *
 * \return An expression pointer
 */
Eptr read(std::istream &stream);
