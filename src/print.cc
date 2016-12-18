/**
 * \file
 * \brief
 * \author    Chris Smeele
 * \copyright Copyright (c) 2016, Chris Smeele
 * \license   TBD, do not redistribute.
 */
#include "print.hh"

void print(const Expr &expr, int depth) {
    std::string indent;
    indent.resize(depth, ' ');

    std::cout << expr.repr() << "\n";
}
