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

struct Symbol {
    Eptr expr;
    Fptr function;
};

class Env {

    std::map<std::string, Symbol> symbols;
    EnvPtr parent;

public:
    Symbol &lookup(const std::string &name);

    void set(const std::string &name, const Symbol &value);
    void set(const std::string &name, Eptr expr);
    void set(const std::string &name, Fptr function);

    Env(EnvPtr parent = nullptr);
};
