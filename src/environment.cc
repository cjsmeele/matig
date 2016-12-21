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


void Env::setHere(const std::string &name, Eptr expr) {
    symbols[name] = expr;
}

void Env::setDeepest(const std::string &name, Eptr expr) {
    auto it = symbols.find(name);
    if (it == symbols.end()) {
        if (parent)
            parent->setDeepest(name, expr);
        else 
            setHere(name, expr);
    } else {
        setHere(name, expr);
    }
}

void Env::setHere(const std::string &name, Fptr func) {
    setHere(name, std::make_shared<FuncExpr>(func));
}
void Env::setDeepest(const std::string &name, Fptr func) {
    setDeepest(name, std::make_shared<FuncExpr>(func));
}

Eptr Env::lookup(const std::string &name) {
    auto it = symbols.find(name);
    if (it == symbols.end()) {
        if (parent)
            return parent->lookup(name);
        else 
            throw SymbolNotFound(name);
    } else {
        return it->second;
    }
}

Env::Env(EnvPtr parent)
    : parent(parent) {

    if (!parent) {
        registerBuiltinFunctions(*this);

        setHere("nil",             std::make_shared<SymbolExpr>("nil"));
        setHere("t",               std::make_shared<SymbolExpr>("t"));
        setHere("*matig-version*", std::make_shared<NumericExpr>(0));
        setHere("*magic*",         std::make_shared<NumericExpr>(539));
    }
}
