#include "expression.hh"
#include "function.hh"

std::string ListExpression::repr() const {
    if (children.size()) {
        std::string s = "(";
        for (const auto &child : children)
            s += child->repr() + " ";
        if (s[s.length()-1] == ' ')
            s.erase(s.end()-1);
        s += ")";
        return s;
    } else {
        return "nil";
    }
}

Eptr ListExpression::eval(Environment &env) const {
    if (children.size()) {
        auto &first = children[0];

        if (first.get()->type() != Expression::Type::SYMBOL)
            throw std::runtime_error("First list element is not a symbol");
        auto symExpr = static_cast<SymbolExpression*>(first.get());

        auto sym = env.lookup(symExpr->repr());

        if (!sym.asFunction)
            throw std::runtime_error("Symbol's function slot is empty");
            
        std::vector<Eptr> parameters;

        for (auto it = children.begin() + 1; it != children.end(); it++)
            parameters.push_back((*it)->eval(env));

        return (*sym.asFunction)(std::move(parameters), env);

    } else {
        return std::make_unique<ListExpression>();
    }
}
