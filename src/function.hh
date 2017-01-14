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

    Fptr func;

public:
    Type type() const override { return Type::FUNC; }
    std::string repr() const override;
    Eptr eval(EnvPtr env) override;

    std::string getDoc(const std::string &exprName) const;

    Fptr getValue() { return func; }

    FuncExpr(Fptr func)
        : func(func)
        { }
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
        std::vector<ParamSpec> keyValue;
        std::string rest;

        bool haveRest() const { return rest.length(); }

        Signature(std::vector<ParamSpec> positional = { },
                  std::vector<ParamSpec> keyValue = { },
                  std::string rest = "");
    };

private:
    Signature signature;
    bool special;
    std::string doc;

    std::unique_ptr<Env> env;

protected:
    virtual Eptr operator()(Elist  positional,
                            Emap   keyValue,
                            Elist  rest,
                            EnvPtr env) const = 0;

public:
    bool isSpecial() const { return special; }
    std::string getSynopsis(const std::string &exprName = "<func>") const;
    std::string getDoc(const std::string &exprName = "<func>") const {
        return getSynopsis(exprName) + "\n" + doc;
    }

    Eptr call(Elist parameters, EnvPtr env) const;

    Func(const Signature &signature,
         bool special,
         const std::string &doc)
        : signature(signature),
          special(special),
          doc(doc)
        { }
};

class FuncC : public Func {

    typedef std::function<Eptr(Elist,
                               Emap,
                               Elist,
                               EnvPtr)> Ftype;

    Ftype func;

public:
    Eptr operator()(Elist  positional,
                    Emap   keyValue,
                    Elist  rest,
                    EnvPtr env) const override {

        return func(positional, keyValue, rest, env);
    }

    FuncC(const std::vector<ParamSpec> &positional,
          const std::vector<ParamSpec> &keyValue,
          const std::string &restName,
          const std::string &doc,
          bool special,
          Ftype func)
        : Func(Signature{positional, keyValue, restName},
               special,
               doc),
          func(func)
        { }
};

class FuncLisp : public Func {

    Elist body;

    EnvPtr context;

public:
    Eptr operator()(Elist  positional,
                    Emap   keyValue,
                    Elist  rest,
                    EnvPtr env) const override {

        if (!context)
            throw LogicError("Null Lisp function context");

        EnvPtr evalCtx = std::make_shared<Env>(context);

        // TODO: make env and bind parameters.

        Eptr result;

        for (auto expr : body)
            result = expr->eval(evalCtx);

        if (!result)
            result = std::make_shared<SymbolExpr>("nil");

        return result;
    }

    FuncLisp(EnvPtr context,
             const Signature &sig,
             const std::string &doc,
             bool special,
             Elist body)
        : Func(sig, special, doc),
          body(body),
          context(context)
        { }
};

void registerBuiltinFunctions(Env &env);
