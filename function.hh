#pragma once

#include "expression.hh"

class Function {
public:
    virtual Eptr operator()(std::vector<Eptr> parameters,
                            Environment &env) = 0;

    virtual ~Function() = default;
};

class FunctionBuiltin : public Function {
    typedef std::function<Eptr(std::vector<Eptr>, Environment&)> F;

    F func;

public:
    Eptr operator()(std::vector<Eptr> parameters,
                    Environment &env) {

        return func(std::move(parameters), env);
    }

    FunctionBuiltin(F func)
        : func(func) { }
};

class FunctionLisp : public Function {

public:
    Eptr operator()(std::vector<Eptr> parameters,
                    Environment &env) {
        return std::make_unique<NumericExpression>(42);
    }
};
