/**
 * \file
 * \brief     Expression classes.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2016, Chris Smeele
 * \license   MIT, see LICENSE.
 */
#pragma once

#include "common.hh"
#include "environment.hh"

#include <vector>

class Expr;
typedef std::shared_ptr<Expr> Eptr;
typedef std::vector<Eptr> Elist;

/**
 * \brief S-Expression type.
 */
class Expr : public std::enable_shared_from_this<Expr> {
public:
    enum class Type {
        NUMERIC,
        STRING,
        SYMBOL,
        CONS,
        FUNC,
    };

    virtual Type type() const = 0;
    virtual bool isNil() const { return false; }

    /**
     * \brief Get the textual representation of the expression.
     */
    virtual std::string repr() const = 0;

    /**
     * \brief Evaluate the expression.
     *
     * \return The evaluation result.
     */
    virtual Eptr eval(EnvPtr env) = 0;

    /**
     * \brief Quote an expression.
     *
     * \param n The amount of quote levels to add
     *
     * \return An expression containing n levels of `(quote ...)`,
     *         containing this expression.
     */
    Eptr quote(int n = 1) __attribute__((warn_unused_result));

    /**
     * \brief Get documentation on an expression.
     *
     * \return A string describing the expression.
     */
    virtual std::string getDoc() const { return ""; };

    virtual ~Expr() = default;
};

/**
 * \brief Atom Expression type.
 */
class AtomExpr : public Expr { };

/**
 * \brief Numeric atom Expression type.
 */
class NumericExpr : public AtomExpr {

    int64_t value;

public:
    Type type() const override { return Type::NUMERIC; }

    std::string repr() const override {
        return std::to_string(value);
    }

    int64_t getValue() const { return value; }
    
    Eptr eval(EnvPtr env) override {
        return shared_from_this();
    }

    NumericExpr(int64_t value)
        : value(value) { }
};

/**
 * \brief String atom Expression type.
 */
class StringExpr : public AtomExpr {

    std::string value;

public:
    Type type() const override { return Type::STRING; }

    std::string repr() const override {
        // TODO: Escaping.
        return std::string("\"") + value + '"';
    }

    const std::string &getValue() const { return value; }
          std::string &getValue()       { return value; }

    Eptr eval(EnvPtr env) override {
        return shared_from_this();
    }

    StringExpr(const std::string &value)
        : value(value) { }
};

/**
 * \brief Symbol atom Expression type.
 */
class SymbolExpr : public AtomExpr {

    std::string value;

public:
    Type type() const override { return Type::SYMBOL; }

    bool isNil() const override { return value == "nil"; }

    std::string repr() const override {
        return value;
    }

    const std::string &getValue() const { return value; }
          std::string &getValue()       { return value; }

    Eptr eval(EnvPtr env) override {
        Eptr expr = env->lookup(value);
        if (!expr)
            throw ProgramError("Symbols value as expression is void");
        return expr;
    }

    SymbolExpr(const std::string &value)
        : value(value) { }
};

/**
 * \brief Cons Expression type.
 */
class ConsExpr : public Expr {

    Eptr car;
    Eptr cdr;

    /**
     * \brief Cons iterator.
     *
     * This, in combination with ConsExpr::begin() and ConsExpr::end()
     * allow iterating over a linked-list using for(:).
     */
    template<class T>
    struct Iterator {
        T *current;
        const Iterator &operator++() {
            if (!current->isListItem())
                throw ProgramError("Looping through non-list");

            if (current->cdr->isNil())
                current = nullptr;
            else
                current = static_cast<ConsExpr*>(current->cdr.get());

            return *this;
        }

        template<class T2>
        bool operator!=(const Iterator<T2> &it2) const {
            return it2.current != current;
        }
        T *operator*() {
            return current;
        }
        Iterator(T *cons)
            : current(cons)
            { }
    };

public:
    Type type() const override { return Type::CONS; }

    std::string repr() const override;

    /**
     * \brief Check if this cons can be approached as an item in a
     *        linked list.
     *
     * A true return value indicates that cdr is either a ConsExpr or
     * nil.
     */
    bool isListItem() const {
        return (cdr->type() == Expr::Type::CONS
                || cdr->isNil());
    }

    /**
     * \brief Check if this cons and all linked cdrs form a linked
     *        list.
     *
     * When true, it is safe to iterate over this list with a ranged
     * for loop.
     */
    bool isList() const;

    std::vector<Eptr> asList();

    const Eptr &getCar() const { return car; }
          Eptr &getCar()       { return car; }
    const Eptr &getCdr() const { return cdr; }
          Eptr &getCdr()       { return cdr; }

    Eptr eval(EnvPtr env) override;

    Iterator<ConsExpr> begin();
    Iterator<ConsExpr> end();
    Iterator<const ConsExpr> begin() const;
    Iterator<const ConsExpr> end() const;

    ConsExpr(const Eptr car = nullptr,
             const Eptr cdr = nullptr)
        : car(car),
          cdr(cdr)
        { }
};
