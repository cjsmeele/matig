#pragma once

#include "common.hh"
#include "environment.hh"

#include <vector>

class Expr;
typedef std::shared_ptr<Expr> Eptr;
typedef std::vector<Eptr> Elist;

class Expr {
public:
    enum class Type {
        NUMERIC,
        STRING,
        SYMBOL,
        CONS,
    };
    virtual Type type() const = 0;
    virtual bool isNil() const { return false; }
    virtual std::string repr() const = 0;
    virtual Eptr eval(Environment &env) const = 0;
    virtual ~Expr() = default;
};

class AtomExpr : public Expr { };

class NumericExpr : public AtomExpr {

    int64_t value;

public:
    Type type() const override { return Type::NUMERIC; }

    std::string repr() const override {
        return std::to_string(value);
    }

    int64_t getValue() const { return value; }
    
    Eptr eval(Environment &env) const override {
        return std::make_unique<NumericExpr>(value);
    }

    NumericExpr(int64_t value)
        : value(value) { }
};

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

    Eptr eval(Environment &env) const override {
        return std::make_unique<StringExpr>(value);
    }

    StringExpr(const std::string &value)
        : value(value) { }
};

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

    Eptr eval(Environment &env) const override {
        // TODO: Lookup.
        return std::make_unique<SymbolExpr>(value);
    }

    SymbolExpr(const std::string &value)
        : value(value) { }
};

class ConsExpr : public Expr {

    Eptr car;
    Eptr cdr;

    template<class T>
    struct Iterator {
        T *current;
        const Iterator &operator++() {
            if (!current->isList())
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
     * \brief Check if this cons can be approached as a linked list.
     *
     * A true return value guarantees that cdr is either a ConsExpr or
     * nullptr.
     */
    bool isList() const;

    const Eptr &getCar() const { return car; }
          Eptr &getCar()       { return car; }
    const Eptr &getCdr() const { return cdr; }
          Eptr &getCdr()       { return cdr; }

    Eptr eval(Environment &env) const override;

    Iterator<ConsExpr> begin();
    Iterator<const ConsExpr> begin() const;
    Iterator<const ConsExpr> end() const;

    ConsExpr(const Eptr car = nullptr,
             const Eptr cdr = nullptr)
        : car(car),
          cdr(cdr)
        { }
};
