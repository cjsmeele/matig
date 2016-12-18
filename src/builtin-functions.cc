/**
 * \file
 * \brief     Builtin functions.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2016, Chris Smeele
 * \license   TBD, do not redistribute.
 */
#include "function.hh"

#include <iostream>

#define MATIG_FUNNAME(name)    cfun_ ## name
#define MATIG_FUNOBJ(name)  fo_cfun_ ## name

#define DEFUN(name) \
    static Eptr MATIG_FUNNAME(name) (Elist parameters, Env &env); \
    static FunctionC MATIG_FUNOBJ(name) (MATIG_FUNNAME(name));          \
    static Eptr MATIG_FUNNAME(name) (Elist parameters, Env &env)

DEFUN(print) {
    // TODO: Function signatures.
    if (parameters.size() != 1)
        throw ProgramError("Incorrect parameter count");

    std::cout << parameters.at(0)->repr() << "\n";
    return std::move(parameters.at(0));
}

DEFUN(opPlus) {
    int64_t result = 0;
    for (const auto &expr : parameters) {
        if (expr.get()->type() != Expr::Type::NUMERIC)
            throw ProgramError("Parameter is not numeric");
        auto numExpr = static_cast<const NumericExpr*>(expr.get());

        result += numExpr->getValue();
    }
    return std::make_shared<NumericExpr>(result);
}

void registerBuiltinFunctions(Env &env) {
    env.set("print", MATIG_FUNOBJ(print));
    env.set("+",     MATIG_FUNOBJ(opPlus));
}
