#include "environment.hh"
#include "function.hh"

#include <iostream>

Symbol &Environment::lookup(const std::string &name) {
    auto it = symbols.find(name);
    if (it == symbols.end()) {
        if (parent)
            return parent->lookup(name);
        else 
            throw std::runtime_error("Symbol does not exist");
    } else {
        return it->second;
    }
}

void Environment::set(const std::string &name, const Symbol &value) {
    symbols[name] = value;
}

Environment::Environment(Environment *parent)
    : parent(parent) {

    // XXX This is all temporary hacks.

    static FunctionBuiltin f([](std::vector<Eptr> parameters, Environment &env) {
            std::cout << parameters.at(0)->repr() << "\n";
            return std::move(parameters.at(0));
        });

    static FunctionBuiltin f2([](std::vector<Eptr> parameters, Environment &env) {
            int64_t result = 0;
            for (const auto &expr : parameters) {
                if (expr.get()->type() != Expression::Type::NUMERIC)
                    throw std::runtime_error("Parameter is not numeric");
                auto numExpr = static_cast<const NumericExpression*>(expr.get());

                result += numExpr->getValue();
            }
            return std::make_unique<NumericExpression>(result);
        });
    
    set("print", { nullptr, &f });
    set("+",     { nullptr, &f2 });
}
