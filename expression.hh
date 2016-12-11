#pragma once

#include "environment.hh"

#include <memory>
#include <vector>

class Expression;
typedef std::unique_ptr<const Expression> Eptr;

class Expression {
public:
    enum class Type {
        NUMERIC,
        STRING,
        SYMBOL,
        LIST,
    };
    virtual Type type() const = 0;
    virtual std::string repr() const = 0;
    virtual Eptr eval(Environment &env) const = 0;
    virtual ~Expression() = default;
};

class AtomExpression : public Expression { };

class NumericExpression : public AtomExpression {

    int64_t value;

public:
    Type type() const { return Type::NUMERIC; }

    std::string repr() const override {
        return std::to_string(value);
    }

    int64_t getValue() const {
        return value;
    }
    
    Eptr eval(Environment &env) const override {
        return std::make_unique<NumericExpression>(value);
    }

    NumericExpression(int64_t value)
        : value(value) { }
};

class StringExpression : public AtomExpression {

    std::string value;

public:
    Type type() const { return Type::STRING; }

    std::string repr() const override {
        // TODO: Escaping.
        return std::string("\"") + value + '"';
    }
    Eptr eval(Environment &env) const override {
        return std::make_unique<StringExpression>(value);
    }

    StringExpression(const std::string &value)
        : value(value) { }
};

class SymbolExpression : public AtomExpression {

    std::string value;

public:
    Type type() const { return Type::SYMBOL; }

    std::string repr() const override {
        return value;
    }
    Eptr eval(Environment &env) const override {
        return std::make_unique<SymbolExpression>(value);
    }

    SymbolExpression(const std::string &value)
        : value(value) { }
};

class ListExpression : public Expression {

    std::vector<Eptr> children;

public:
    Type type() const { return Type::LIST; }

    std::string repr() const override;
    Eptr eval(Environment &env) const override;

    ListExpression()
        : children{ } { }

    ListExpression(std::vector<Eptr> children)
        : children(std::move(children)) { }
};
