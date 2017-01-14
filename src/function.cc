/**
 * \file
 * \brief
 * \author    Chris Smeele
 * \copyright Copyright (c) 2016, 2017, Chris Smeele
 * \license   MIT, see LICENSE.
 */
#include "function.hh"

#include <algorithm>

std::string FuncExpr::repr() const {
    return "<func>";
}

Eptr FuncExpr::eval(EnvPtr env) {
    // TODO.
    return std::make_shared<SymbolExpr>("nil");
}

std::string FuncExpr::getDoc(const std::string &exprName) const {
    return func->getDoc(exprName);
}


Func::Signature::Signature(std::vector<ParamSpec> positional,
                           std::vector<ParamSpec> keyValue,
                           std::string rest)
    : positional(positional),
      keyValue(keyValue),
      rest(rest) {

    bool haveOptional = false;
    for (const auto &p : positional) {
        if (p.defaultValue)
            haveOptional = true;
        else if (haveOptional)
            throw ProgramError("Default parameters must come last in function declaration");
    }
}

std::string Func::getSynopsis(const std::string &exprName) const {
    std::string s = "("s + exprName;
    for (const auto &p : signature.positional) {
        std::string name = p.name;
        std::transform(name.begin(),
                       name.end(),
                       name.begin(),
                       toupper);

        if (p.defaultValue) {
            s += " (" + name + " " + p.defaultValue->repr() + ")";
        } else {
            s += " " + name;
        }
    }

    if (signature.haveRest()) {
        auto name = signature.rest;
        std::transform(name.begin(),
                       name.end(),
                       name.begin(),
                       toupper);

        s += " &rest " + name;
    }

    return s + ")";
}

Eptr Func::call(Elist parametersIn, EnvPtr env) const {
    unsigned required = std::count_if(signature.positional.begin(),
                                      signature.positional.end(),
                                      [](ParamSpec p) {
                                          return !p.defaultValue; });

    unsigned optional = signature.positional.size() - required;

    unsigned minPositional = required;
    unsigned maxPositional = required + optional;

    if (parametersIn.size() < (size_t)minPositional)
        throw ProgramError("Function expects at least "s
                           + std::to_string(minPositional)
                           + (minPositional == 1 ? " parameter, " :" parameters, ")
                           + std::to_string(parametersIn.size())
                           + " given");

    if (!signature.haveRest()
        && parametersIn.size() > (size_t)maxPositional)

        throw ProgramError("Function expects at most "s
                           + std::to_string(maxPositional)
                           + (maxPositional == 1 ? " parameter, " : " parameters, ")
                           + std::to_string(parametersIn.size())
                           + " given");

    Elist positionals;
    Emap  keyValues;
    Elist rest;

    for (unsigned i = 0; i < maxPositional; i++) {
        auto value = i < parametersIn.size()
                         ? isSpecial()
                             ? parametersIn[i]
                             : parametersIn[i]->eval(env)
                         // Default parameter values are always evaluated.
                         : signature.positional[i].defaultValue;

        if (!value)
            value = std::make_shared<SymbolExpr>("nil");

        positionals.push_back(value);
    }

    // TODO: Key-value parameters.

    for (unsigned i = positionals.size(); i < parametersIn.size(); i++) {
        rest.push_back(isSpecial()
                       ? parametersIn[i]
                       : parametersIn[i]->eval(env));
    }

    return (*this)(positionals, keyValues, rest, env);
}


Eptr FuncLisp::operator()(Elist  positional,
                          Emap   keyValue,
                          Elist  rest,
                          EnvPtr env) const {

    if (!context)
        throw LogicError("Null Lisp function context");

    EnvPtr evalCtx = std::make_shared<Env>(context);

    auto sig = getSignature();
    for (unsigned i = 0; i < sig.positional.size(); i++) {
        // Set positional parameters in env.
        evalCtx->setHere(sig.positional[i].name, positional[i]);
    }

    if (sig.haveRest()) {
        // Set rest.
        evalCtx->setHere(sig.rest, ConsExpr::fromList(rest));
    }

    Eptr result;

    for (auto expr : body)
        result = expr->eval(evalCtx);

    if (!result)
        result = std::make_shared<SymbolExpr>("nil");

    return result;
}
