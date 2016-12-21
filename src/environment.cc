/**
 * \file
 * \brief
 * \author    Chris Smeele
 * \copyright Copyright (c) 2016, Chris Smeele
 * \license   MIT, see LICENSE.
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

void Env::set(const std::string &name, Eptr expr) {
    symbols[name].expr = expr;
}

void Env::set(const std::string &name, Fptr function) {
    symbols[name].function = function;
}

Env::Env(EnvPtr parent)
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
            sym.expr = nilExpr;
            sym.function = nullptr;
            set("nil", sym);
        }
        {
            Symbol sym;
            auto expr = std::make_shared<SymbolExpr>("t");
            sym.expr = expr;
            sym.function = nullptr;
            set("t", sym);
        }
        {
            Symbol sym;
            auto expr = std::make_shared<NumericExpr>(539);
            sym.expr = expr;
            sym.function = nullptr;
            set("*magic*", sym);
        }
    }
}
