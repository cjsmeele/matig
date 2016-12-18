#pragma once

#include "common.hh"
#include "expression.hh"

class Function {
public:
    virtual Eptr operator()(std::vector<Eptr> parameters,
                            Environment &env) const = 0;

    virtual ~Function() = default;
};

/**
 * \brief A special form implemented in C/C++.
 */
class FunctionSpecial : public Function {
    typedef std::function<Eptr(std::vector<Eptr>, Environment&)> F;

    F func;

public:
    Eptr operator()(std::vector<Eptr> parameters,
                    Environment &env) const {

        return func(std::move(parameters), env);
    }

    FunctionSpecial(F func)
        : func(func) { }
};

/**
 * \brief A function implemented in C/C++.
 */
class FunctionC : public Function {
    typedef std::function<Eptr(std::vector<Eptr>, Environment&)> F;

    F func;

public:
    Eptr operator()(std::vector<Eptr> parameters,
                    Environment &env) const {

        return func(std::move(parameters), env);
    }

    FunctionC(F func)
        : func(func) { }
};

/**
 * \brief A function implemented in Lisp.
 */
class FunctionLisp : public Function {

public:
    Eptr operator()(std::vector<Eptr> parameters,
                    Environment &env) const {
        return std::make_unique<NumericExpr>(42);
    }
};

void registerBuiltinFunctions(Environment &env);
