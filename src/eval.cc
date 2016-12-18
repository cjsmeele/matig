#include "eval.hh"
#include "environment.hh"

Eptr eval(Expr &expr) {
    Environment env;
    return expr.eval(env);
}
