#include "print.hh"

void print(const Expression &expr, int depth) {
    std::string indent;
    indent.resize(depth, ' ');

    std::cout << expr.repr() << "\n";
}
