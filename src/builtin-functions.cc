/**
 * \file
 * \brief     Builtin functions.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2016, Chris Smeele
 * \license   MIT, see LICENSE.
 */
#include "function.hh"

#include <iostream>

void registerBuiltinFunctions(Env &env) {

    env.set("quote", std::make_shared<FuncC>(FuncC(
        { {"thing"} }, "", "",
        "Return expression THING without evaluating it.",
        true,
        [](Elist parameters, EnvPtr env) {
            return std::move(parameters.at(0));
        })));

    env.set("print", std::make_shared<FuncC>(FuncC(
        { {"thing"} }, "", "",
        "Print the textual representation of THING and return it.",
        false,
        [](Elist parameters, EnvPtr env) {
            std::cout << parameters.at(0)->repr() << "\n";
            return std::move(parameters.at(0));
        })));
    
    env.set("let", std::make_shared<FuncC>(FuncC(
        { {"decls"} }, "", "body",
        "Bind DECLS in a new environment, and evaluate BODY in the new environment.",
        true,
        [](Elist parameters, EnvPtr env) {

            // The environment in which we will eval our body.
            auto subEnv = std::make_shared<Env>(env);

            auto declsExpr = parameters.at(0);
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
                        sym.expr = declList[1]->eval(env);
                        subEnv->set(symExpr->getValue(), sym);

                    } else if (car->type() == Expr::Type::SYMBOL) {
                        // (sym) declaration (sym is set to nil).
                        auto symExpr = static_cast<SymbolExpr*>(car.get());
                        Symbol sym;
                        sym.expr = std::make_shared<SymbolExpr>("nil");
                        subEnv->set(symExpr->getValue(), sym);
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
        })));

    env.set("+", std::make_shared<FuncC>(FuncC(
        { }, "", "rest",
        "Sum all numerics in REST.",
        false,
        [](Elist parameters, EnvPtr env) {
            int64_t result = 0;
            for (const auto &expr : parameters) {
                if (expr.get()->type() != Expr::Type::NUMERIC)
                    throw ProgramError("Parameter is not numeric");
                auto numExpr = static_cast<const NumericExpr*>(expr.get());

                result += numExpr->getValue();
            }
            return std::make_shared<NumericExpr>(result);
        })));
}
