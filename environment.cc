#include "environment.hh"
#include "function.hh"

#include <iostream>

Symbol &Environment::lookup(const std::string &name) {
    auto it = symbols.find(name);
    if (it == symbols.end()) {
        if (parent)
            return parent->lookup(name);
        else 
            throw std::runtime_error("Symbol does not exist");
    } else {
        return it->second;
    }
}

void Environment::set(const std::string &name, const Symbol &value) {
    symbols[name] = value;
}

void Environment::set(const std::string &name, const Function &value) {
    symbols[name].asFunction = &value;
}

Environment::Environment(Environment *parent)
    : parent(parent) {

    // XXX
    registerBuiltinFunctions(*this);
}
