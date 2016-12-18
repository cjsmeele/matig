#include "eval.hh"
#include "environment.hh"

Eptr eval(const Expr &expr) {
    Environment env;
    return expr.eval(env);
}
