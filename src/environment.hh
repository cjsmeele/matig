/**
 * \file
 * \brief     Environment / symbol scope.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2016, Chris Smeele
 * \license   MIT, see LICENSE.
 */
#pragma once

#include "common.hh"
#include <map>

class Expr;
typedef std::shared_ptr<Expr> Eptr;

class Func;
typedef std::shared_ptr<Func> Fptr;

class Env;
typedef std::shared_ptr<Env> EnvPtr;

class Env {
public:
    class SymbolNotFound : public ProgramError {
    public:
        SymbolNotFound(const std::string &name)
            : ProgramError("Symbol '"s + name + "' not found")
            { }
    };

private:
    std::map<std::string, Eptr> symbols;
    EnvPtr parent;

public:
    Eptr lookup(const std::string &name);

    void setHere(const std::string &name, Eptr expr);
    void setDeepest(const std::string &name, Eptr expr);

    void setHere(const std::string &name, Fptr func);
    void setDeepest(const std::string &name, Fptr func);

    Env(EnvPtr parent = nullptr);
};
