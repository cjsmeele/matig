#include "function.hh"

#include <iostream>

#define MATIG_FUNNAME(name)    cfun_ ## name
#define MATIG_FUNOBJ(name)  fo_cfun_ ## name

#define DEFUN(name) \
    static Eptr MATIG_FUNNAME(name) (Elist parameters, Environment &env); \
    static FunctionC MATIG_FUNOBJ(name) (MATIG_FUNNAME(name));          \
    static Eptr MATIG_FUNNAME(name) (Elist parameters, Environment &env)

DEFUN(print) {
    std::cout << parameters.at(0)->repr() << "\n";
    return std::move(parameters.at(0));
}

DEFUN(opPlus) {
    int64_t result = 0;
    for (const auto &expr : parameters) {
        if (expr.get()->type() != Expression::Type::NUMERIC)
            throw std::runtime_error("Parameter is not numeric");
        auto numExpr = static_cast<const NumericExpression*>(expr.get());

        result += numExpr->getValue();
    }
    return std::make_unique<NumericExpression>(result);
}

void registerBuiltinFunctions(Environment &env) {
    env.set("print", MATIG_FUNOBJ(print));
    env.set("+",     MATIG_FUNOBJ(opPlus));
}