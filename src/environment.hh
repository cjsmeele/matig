#pragma once

#include "common.hh"
#include <map>

class Expr;
typedef std::shared_ptr<Expr> Eptr;
class Function;

struct Symbol {
    // std::string asSymbol;
    // std::string asString;
    // int64_t     asInt;
    Eptr      asExpr;
    Function *asFunction;
};

class Env {
    std::map<std::string, Symbol> symbols;
    Env *parent;
public:
    Symbol &lookup(const std::string &name);

    void set(const std::string &name, const Symbol &value);
    void set(const std::string &name, Function &value);

    // Env(Env *parent = nullptr)
    //     : parent(parent) {
    // }
    Env(Env *parent = nullptr);
};
