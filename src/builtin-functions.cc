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

#define DEFSPECIAL(name) \
    static Eptr MATIG_FUNNAME(name) (Elist parameters, Env &env); \
    static SpecOp MATIG_FUNOBJ(name) (MATIG_FUNNAME(name));          \
    static Eptr MATIG_FUNNAME(name) (Elist parameters, Env &env)

DEFSPECIAL(quote) {
    if (parameters.size() != 1)
        throw ProgramError("Incorrect parameter count (expected 1, got "s
                           + std::to_string(parameters.size()) + ")");

    return std::move(parameters.at(0));
}

DEFUN(print) {
    if (parameters.size() != 1)
        throw ProgramError("Incorrect parameter count (expected 1, got "s
                           + std::to_string(parameters.size()) + ")");

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
    env.set("quote", MATIG_FUNOBJ(quote));
    env.set("print", MATIG_FUNOBJ(print));
    env.set("+",     MATIG_FUNOBJ(opPlus));
}
