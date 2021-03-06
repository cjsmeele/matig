/**
 * \file
 * \brief
 * \author    Chris Smeele
 * \copyright Copyright (c) 2016, 2017, Chris Smeele
 * \license   MIT, see LICENSE.
 */
#include "expression.hh"
#include "function.hh"

Eptr Expr::quote(int count) {

    Eptr current = shared_from_this();

    for (int i = 0; i < count; i++) {
        auto quoteCons = std::make_shared<ConsExpr>();
        auto innerCons = std::make_shared<ConsExpr>();
        innerCons->getCar() = current;
        innerCons->getCdr() = std::make_shared<SymbolExpr>("nil");
        quoteCons->getCar() = std::make_shared<SymbolExpr>("quote");
        quoteCons->getCdr() = innerCons;

        current = quoteCons;
    }

    return current;
}

bool ConsExpr::isList() const {
    if (!isListItem())
        return false;

    for (const auto &cons : *this) {
        if (!cons->cdr->isNil()
            && cons->cdr->type() != Expr::Type::CONS)
            return false;
    }

    return true;
}

Elist ConsExpr::asList() {
    std::vector<Eptr> list;
    for (const auto &cons : *this)
        list.push_back(cons->car);

    return std::move(list);
}

Eptr ConsExpr::fromList(const Elist &list) {

    if (!list.size())
        return std::move(std::make_shared<SymbolExpr>("nil"));

    auto rootCons = std::make_shared<ConsExpr>();
    std::shared_ptr<ConsExpr> currentCons = rootCons;

    for (auto &expr : list) {
        if (currentCons->getCar()) {
            currentCons->getCdr() = std::make_shared<ConsExpr>();
            currentCons = std::static_pointer_cast<ConsExpr>(currentCons->getCdr());
        }
        currentCons->getCar() = expr;
    }
    currentCons->getCdr() = std::make_shared<SymbolExpr>("nil");

    if (!currentCons->getCar())
        currentCons->getCdr() = std::make_shared<SymbolExpr>("nil");

    return std::move(rootCons);
}

Eptr ConsExpr::operator[](size_t i) const {
    size_t j = 0;
    for (const auto &cons : *this) {
        if (j == i)
            return cons->car;
        j++;
    }
    throw LogicError("Cons list index out of range");
}

std::string ConsExpr::repr() const {

    if (!car)
        throw LogicError("Null car");
    if (!cdr)
        throw LogicError("Null cdr");

    if (car->type() == Expr::Type::SYMBOL
        && static_cast<SymbolExpr*>(car.get())->getValue() == "quote"
        && cdr->type() == Expr::Type::CONS
        && static_cast<ConsExpr*>(cdr.get())->cdr->isNil()) {

        // This cons is a (quote ...) form with a single parameter.

        return "'"s + static_cast<ConsExpr*>(cdr.get())->car->repr();
    }

    if (isNil())
        return "nil";

    std::string s = "(";

    if (isListItem()) {
        for (const ConsExpr *cons : *this) {
            s += cons->car ? cons->car->repr() : "nil";

            if (cons->isListItem()) {
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

Eptr ConsExpr::eval(EnvPtr env) {
    if (!car)
        throw LogicError("Null car");
    if (!cdr)
        throw LogicError("Null cdr");

    if (!isListItem())
        throw ProgramError("Evaling non-list cons");

    if (car->type() != Type::SYMBOL)
        throw ProgramError("First list element <"s + car->repr() + "> is not a symbol");

    auto symExpr = static_cast<const SymbolExpr*>(car.get());

    Eptr sym = env->lookup(symExpr->getValue());

    if (!sym || sym->type() != Expr::Type::FUNC)
        throw ProgramError("Symbol does not point to a function");

    Fptr func = static_cast<FuncExpr*>(sym.get())->getValue();
            
    Elist parameters;

    // Loop through the cars in the parameter chain.

    if (!cdr->isNil()) {
        // Since this->isListItem(), cdr must be a cons.
        const ConsExpr *paramsCons = static_cast<ConsExpr*>(cdr.get());

        for (const ConsExpr *cons : *paramsCons)
            parameters.push_back(std::move(cons->car));
    }

    return func->call(std::move(parameters), env);
}

ConsExpr::Iterator<ConsExpr>       ConsExpr::begin()       { return Iterator<ConsExpr>{this};    }
ConsExpr::Iterator<ConsExpr>       ConsExpr::end()         { return Iterator<ConsExpr>{nullptr}; }
ConsExpr::Iterator<const ConsExpr> ConsExpr::begin() const { return Iterator<const ConsExpr>(this);    }
ConsExpr::Iterator<const ConsExpr> ConsExpr::end()   const { return Iterator<const ConsExpr>{nullptr}; }
