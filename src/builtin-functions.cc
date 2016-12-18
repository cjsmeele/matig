/**
 * \file
 * \brief     Builtin functions.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2016, Chris Smeele
 * \license   TBD, do not redistribute.
 */
#include "function.hh"

#include <iostream>

static Function::Signature sigify(std::vector<std::string> positional,
                                  std::string optional = "",
                                  std::string rest     = "") {

    // WIP.
    return Function::Signature{ std::move(positional), optional, rest };
}

#define MATIG_FUNNAME(name)    cfun_ ## name
#define MATIG_FUNOBJ(name)  fo_cfun_ ## name

#define DEFUN(name, sig)                                          \
    static Eptr MATIG_FUNNAME(name) (Elist parameters, Env &env);     \
    static FunctionC MATIG_FUNOBJ(name) (MATIG_FUNNAME(name), sig);   \
    static Eptr MATIG_FUNNAME(name) (Elist parameters, Env &env)

#define DEFSPECIAL(name, sig)                                     \
    static Eptr MATIG_FUNNAME(name) (Elist parameters, Env &env);    \
    static SpecOp MATIG_FUNOBJ(name) (MATIG_FUNNAME(name), sig);     \
    static Eptr MATIG_FUNNAME(name) (Elist parameters, Env &env)

DEFSPECIAL(quote, sigify({"thing"})) {
    if (parameters.size() != 1)
        throw ProgramError("Incorrect parameter count (expected 1, got "s
                           + std::to_string(parameters.size()) + ")");

    return std::move(parameters.at(0));
}

DEFUN(print, sigify({"thing"})) {
    if (parameters.size() != 1)
        throw ProgramError("Incorrect parameter count (expected 1, got "s
                           + std::to_string(parameters.size()) + ")");

    std::cout << parameters.at(0)->repr() << "\n";
    return std::move(parameters.at(0));
}

DEFSPECIAL(let, sigify({"asdf"})) {
    if (parameters.size() < 1)
        throw ProgramError("Incorrect parameter count (expected 1+, got "s
                           + std::to_string(parameters.size()) + ")");

    // The environment in which we will eval our body.
    Env subEnv(env);

    auto declsExpr = parameters[0];
    if (!declsExpr->isNil()) {
        if (declsExpr->type() != Expr::Type::CONS)
            throw ProgramError("First parameter of let must be a declaration list");

        auto declsCons = static_cast<ConsExpr*>(declsExpr.get());

        if (!declsCons->isList())
            throw ProgramError("First parameter of let must be a declaration list");

        // Define given symbols in subEnv.
        for (auto declCons : *declsCons) {
            auto car = declCons->getCar();
            if (car->type() == Expr::Type::CONS) {
                // ((sym value)) declaration.
                auto declCons = static_cast<ConsExpr*>(car.get());
                if (!declCons->isList())
                    throw ProgramError("Invalid let syntax (1)");

                auto declList = declCons->asList();
                if (declList.size() != 2)
                    throw ProgramError("Invalid let syntax (2)");

                if (declList[0]->type() != Expr::Type::SYMBOL)
                    throw ProgramError("Invalid let syntax (3)");

                auto symExpr = static_cast<SymbolExpr*>(declList[0].get());
                Symbol sym;
                sym.asExpr = declList[1]->eval(env);
                subEnv.set(symExpr->getValue(), sym);

            } else if (car->type() == Expr::Type::SYMBOL) {
                // (sym) declaration (sym is set to nil).
                auto symExpr = static_cast<SymbolExpr*>(car.get());
                Symbol sym;
                sym.asExpr = std::make_shared<SymbolExpr>("nil");
                subEnv.set(symExpr->getValue(), sym);
            } else {
                throw ProgramError("Invalid let syntax (0)");
            }
        }
    }

    Eptr result = nullptr;

    // Evaluate body.
    if (parameters.size() > 1) {
        for (auto it = parameters.begin()+1; it != parameters.end(); ++it)
            result = (*it)->eval(subEnv);
    }

    if (!result)
        result = std::make_shared<SymbolExpr>("nil");

    return std::move(result);
}

DEFUN(opPlus, sigify({}, "rest")) {
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
    env.set("let",   MATIG_FUNOBJ(let));
    env.set("+",     MATIG_FUNOBJ(opPlus));
}
