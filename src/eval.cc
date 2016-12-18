#include "eval.hh"
#include "environment.hh"

Eptr eval(Expr &expr, Env &env) {
    return expr.eval(env);
}
