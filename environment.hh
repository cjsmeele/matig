#pragma once

#include <map>

class Expression;
class Function;

struct Symbol {
    // std::string asSymbol;
    // std::string asString;
    // int64_t     asInt;
    Expression  *asExpression;
    Function    *asFunction;
};

class Environment {
    std::map<std::string, Symbol> symbols;
    Environment *parent;
public:
    Symbol &lookup(const std::string &name);

    void set(const std::string &name, const Symbol &value);

    // Environment(Environment *parent = nullptr)
    //     : parent(parent) {
    // }
    Environment(Environment *parent = nullptr);
};
