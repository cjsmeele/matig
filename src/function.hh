/**
 * \file
 * \brief     Function types.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2016, Chris Smeele
 * \license   MIT, see LICENSE.
 */
#pragma once

#include "common.hh"
#include "expression.hh"

class FuncExpr : public AtomExpr {
public:
    Type type() const override { return Type::FUNC; }
    std::string repr() const override;
    Eptr eval(Env &env) override;
};

class Function {
    // TODO: Function signature.

public:
    struct Signature {
        std::vector<std::string> positional;
        std::string optional;
        std::string restName;
    };

protected:
    Signature sig;

    std::unique_ptr<Env> env;

public:
    virtual bool isSpecial() { return false; }

    virtual Eptr operator()(std::vector<Eptr> parameters,
                            Env &env) const = 0;

    Function(const Signature &sig)
        : sig(sig)
        { }
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

    FunctionC(F func, const Signature &sig)
        : Function(sig),
          func(func)
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

    FunctionLisp(Eptr body, const Signature &sig)
        : Function(sig),
          body(body)
        { }
};

class SpecOp : public FunctionC {
public:
    bool isSpecial() { return true; }

    SpecOp(F func, const Signature &sig)
        : FunctionC(func, sig)
        { }
};

class Macro : public FunctionLisp {
public:
    bool isSpecial() { return true; }

    Macro(Eptr body, const Signature &sig)
        : FunctionLisp(body, sig)
        { }
};

void registerBuiltinFunctions(Env &env);
