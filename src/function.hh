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

class Func;
typedef std::shared_ptr<Func> Fptr;

class FuncExpr : public AtomExpr {
public:
    Type type() const override { return Type::FUNC; }
    std::string repr() const override;
    Eptr eval(EnvPtr env) override;
};

class Func {

public:
    struct ParamSpec {
        std::string name;
        Eptr defaultValue;

        ParamSpec(const std::string &name,
                  Eptr defaultValue = nullptr)
            : name(name),
              defaultValue(defaultValue)
            { }
    };

    struct Signature {
        std::vector<ParamSpec> positional;
        std::string optional;
        std::string rest;
    };

private:
    Signature signature;
    bool special;
    std::string doc;

    std::unique_ptr<Env> env;

public:
    bool isSpecial() { return special; }

    virtual Eptr operator()(std::vector<Eptr> parameters,
                            EnvPtr env) const = 0;

    Func(const Signature &signature,
         bool special,
         const std::string &doc)
        : signature(signature),
          special(special),
          doc(doc)
        { }
};

class FuncC : public Func {

    typedef std::function<Eptr(std::vector<Eptr>, EnvPtr)> Ftype;

    Ftype func;

public:
    Eptr operator()(std::vector<Eptr> parameters,
                    EnvPtr env) const {

        return func(std::move(parameters), env);
    }

    FuncC(const std::vector<ParamSpec> &parameters,
          const std::string &optionalName,
          const std::string &restName,
          const std::string &doc,
          bool special,
          Ftype func)
        : Func(Signature{parameters, optionalName, restName},
               special,
               doc),
          func(func)
        { }
};

class FuncLisp : public Func {

    Eptr body;

    EnvPtr context;

public:
    Eptr operator()(std::vector<Eptr> parameters,
                    EnvPtr env) const {

        return std::make_shared<NumericExpr>(42);
        return body->eval(env);
    }

    FuncLisp(EnvPtr context,
             const Signature &sig,
             const std::string &doc,
             bool special,
             Eptr body)
        : Func(sig, special, doc),
          body(body),
          context(context)
        { }
};

void registerBuiltinFunctions(Env &env);
