/**
 * \file
 * \brief
 * \author    Chris Smeele
 * \copyright Copyright (c) 2016, Chris Smeele
 * \license   TBD, do not redistribute.
 */
#include "expression.hh"
#include "function.hh"

std::string Expr::repr() const {
    std::string s;
    s.resize(quoteLevel, '\'');
    return s + repr2();
}

Eptr Expr::eval(Env &env) {
    if (quoteLevel) {
        quoteLevel--;
        return shared_from_this();
    } else {
        return eval2(env);
    }
}

bool ConsExpr::isList() const {
    return (cdr->type() == Expr::Type::CONS
            || cdr->isNil());
}

std::string ConsExpr::repr2() const {

    if (!car)
        throw LogicError("Null car");
    if (!cdr)
        throw LogicError("Null car");

    if (isNil())
        return "nil";

    std::string s = "(";

    if (isList()) {
        for (const ConsExpr *cons : *this) {
            s += cons->car ? cons->car->repr() : "nil";

            if (cons->isList()) {
                if (!cons->cdr->isNil())
                    s += " ";
            } else {
                s += " . ";
                s += cons->cdr ? cons->cdr->repr() : "nil";
                break;
            }
        }
    } else {
        s += car ? car->repr() : "nil";

        if (cdr)
            s += " . " + cdr->repr();
    }

    return s + ")";
}

Eptr ConsExpr::eval2(Env &env) {
    if (!car)
        throw LogicError("Null car");
    if (!cdr)
        throw LogicError("Null car");

    if (!isList())
        throw ProgramError("Evaling non-list cons");

    if (car->type() != Type::SYMBOL)
        throw ProgramError("First list element is not a symbol");

    auto symExpr = static_cast<const SymbolExpr*>(car.get());

    if (symExpr->getQuoteLevel())
        throw ProgramError("Invalid function call: quoted symbol");

    Symbol &sym = env.lookup(symExpr->getValue());

    if (!sym.asFunction)
        throw ProgramError("Symbol's function slot is empty");
            
    Elist parameters;

    // Evaluate all cars in the parameter chain.

    if (!cdr->isNil()) {
        // Since this->isList(), cdr must be a cons.
        const ConsExpr *paramsCons = static_cast<ConsExpr*>(cdr.get());
        for (const ConsExpr *cons : *paramsCons)
            parameters.push_back(std::move(cons->car->eval(env)));
    }

    return (*sym.asFunction)(std::move(parameters), env);
}

ConsExpr::Iterator<ConsExpr>       ConsExpr::begin()       { return Iterator<ConsExpr>{this};    }
ConsExpr::Iterator<ConsExpr>       ConsExpr::end()         { return Iterator<ConsExpr>{nullptr}; }
ConsExpr::Iterator<const ConsExpr> ConsExpr::begin() const { return Iterator<const ConsExpr>(this);    }
ConsExpr::Iterator<const ConsExpr> ConsExpr::end()   const { return Iterator<const ConsExpr>{nullptr}; }
