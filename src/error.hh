/**
 * \file
 * \brief     Exception types.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2016, Chris Smeele
 * \license   MIT, see LICENSE.
 */
#pragma once

#include <string>
#include <stdexcept>

/**
 * \brief An error in the interpreter, indicating a bug.
 */
class LogicError : public std::logic_error {
public:
    LogicError(const std::string &s)
        : std::logic_error(s) { }
};

/**
 * \brief An error in the executed program.
 */
class ProgramError : public std::runtime_error {
public:
    ProgramError(const std::string &s)
        : std::runtime_error(s) { }
};
