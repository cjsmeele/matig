/**
 * \file
 * \brief     Function types.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2016, Chris Smeele
 * \license   TBD, do not redistribute.
 */
#pragma once

#include "common.hh"
#include "expression.hh"

class Function {
    // TODO: Function signature.

public:
    virtual bool isSpecial() { return false; }

    virtual Eptr operator()(std::vector<Eptr> parameters,
                            Env &env) const = 0;

    virtual ~Function() = default;
};

class FunctionC : public Function {
protected:
    typedef std::function<Eptr(std::vector<Eptr>, Env&)> F;

private:
    F func;

public:
    Eptr operator()(std::vector<Eptr> parameters,
                    Env &env) const {

        return func(std::move(parameters), env);
    }

    FunctionC(F func)
        : func(func)
        { }
};

class FunctionLisp : public Function {

    Eptr body;

public:
    Eptr operator()(std::vector<Eptr> parameters,
                    Env &env) const {

        return std::make_shared<NumericExpr>(42);
        // return body->eval(env);
    }

    FunctionLisp(Eptr body)
        : body(body)
        { }
};

class SpecOp : public FunctionC {
public:
    bool isSpecial() { return true; }

    SpecOp(F func)
        : FunctionC(func)
        { }
};

class Macro : public FunctionLisp {
public:
    bool isSpecial() { return true; }

    Macro(Eptr body)
        : FunctionLisp(body)
        { }
};

void registerBuiltinFunctions(Env &env);
