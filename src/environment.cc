/**
 * \file
 * \brief
 * \author    Chris Smeele
 * \copyright Copyright (c) 2016, Chris Smeele
 * \license   TBD, do not redistribute.
 */
#include "environment.hh"
#include "function.hh"

#include <iostream>

Symbol &Env::lookup(const std::string &name) {
    auto it = symbols.find(name);
    if (it == symbols.end()) {
        if (parent)
            return parent->lookup(name);
        else 
            throw ProgramError("Symbol '"s + name + "' does not exist");
    } else {
        return it->second;
    }
}

void Env::set(const std::string &name, const Symbol &value) {
    symbols[name] = value;
}

void Env::set(const std::string &name, Function &value) {
    symbols[name].asFunction = &value;
}

Env::Env(Env *parent)
    : parent(parent) {

    // XXX This shouldn't be here.
    if (!parent) {
        registerBuiltinFunctions(*this);
        {
            Symbol sym;
            // auto nilExpr = std::make_shared<ConsExpr>();
            // nilExpr->getCar() = nilExpr;
            // nilExpr->getCdr() = nilExpr;
            auto nilExpr = std::make_shared<SymbolExpr>("nil");
            sym.asExpr = nilExpr;
            sym.asFunction = nullptr;
            set("nil", sym);
        }
        {
            Symbol sym;
            auto expr = std::make_shared<SymbolExpr>("t");
            sym.asExpr = expr;
            sym.asFunction = nullptr;
            set("t", sym);
        }
        {
            Symbol sym;
            auto expr = std::make_shared<NumericExpr>(539);
            sym.asExpr = expr;
            sym.asFunction = nullptr;
            set("*magic*", sym);
        }
    }
}
