#include "eval.hh"
#include "environment.hh"

Eptr eval(const Expression &expr) {
    Environment env;
    return expr.eval(env);
}
