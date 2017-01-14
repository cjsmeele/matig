/**
 * \file
 * \brief
 * \author    Chris Smeele
 * \copyright Copyright (c) 2016, Chris Smeele
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
        s += " " + name;
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
                           + (minPositional == 1 ? " parameter, " :" parametersIn, ")
                           + std::to_string(parametersIn.size())
                           + " given");

    if (!signature.haveRest()
        && parametersIn.size() > (size_t)maxPositional)

        throw ProgramError("Function expects at most "s
                           + std::to_string(maxPositional)
                           + (maxPositional == 1 ? " parameter, " : " parametersIn, ")
                           + std::to_string(parametersIn.size())
                           + " given");

    Elist positionals;
    Emap  keyValues;
    Elist rest;

    for (unsigned i = 0; i < maxPositional; i++) {
        if (i < parametersIn.size()) {
            positionals.push_back(parametersIn[i]);
        } else {
            // Fill in optional parameters.
            positionals.push_back(signature.positional[0].defaultValue);
        }
    }

    // TODO: Key-value parameters.

    for (unsigned i = positionals.size(); i < parametersIn.size(); i++) {
        rest.push_back(parametersIn[i]);
    }

    return (*this)(positionals, keyValues, rest, env);
}
