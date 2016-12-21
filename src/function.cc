/**
 * \file
 * \brief     Builtin functions.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2016, Chris Smeele
 * \license   MIT, see LICENSE.
 */
#include "function.hh"

std::string FuncExpr::repr() const {
    return "<func>";
}

Eptr FuncExpr::eval(EnvPtr env) {
    // TODO.
    return std::make_shared<SymbolExpr>("nil");
}
