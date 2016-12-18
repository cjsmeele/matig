#pragma once

#include "common.hh"
#include <map>

class Expr;
class Function;

struct Symbol {
    // std::string asSymbol;
    // std::string asString;
    // int64_t     asInt;
    const Expr  *asExpr;
    const Function    *asFunction;
};

class Environment {
    std::map<std::string, Symbol> symbols;
    Environment *parent;
public:
    Symbol &lookup(const std::string &name);

    void set(const std::string &name, const Symbol &value);
    void set(const std::string &name, const Function &value);

    // Environment(Environment *parent = nullptr)
    //     : parent(parent) {
    // }
    Environment(Environment *parent = nullptr);
};
