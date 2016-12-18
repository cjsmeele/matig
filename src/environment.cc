#include "environment.hh"
#include "function.hh"

#include <iostream>

Symbol &Env::lookup(const std::string &name) {
    auto it = symbols.find(name);
    if (it == symbols.end()) {
        if (parent)
            return parent->lookup(name);
        else 
            throw ProgramError("Symbol '"s + name + "' does not exist");
    } else {
        return it->second;
    }
}

void Env::set(const std::string &name, const Symbol &value) {
    symbols[name] = value;
}

void Env::set(const std::string &name, Function &value) {
    symbols[name].asFunction = &value;
}

Env::Env(Env *parent)
    : parent(parent) {

    // XXX
    registerBuiltinFunctions(*this);
}
