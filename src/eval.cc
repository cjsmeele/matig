/**
 * \file
 * \brief
 * \author    Chris Smeele
 * \copyright Copyright (c) 2016, Chris Smeele
 * \license   MIT, see LICENSE.
 */
#include "eval.hh"
#include "environment.hh"

Eptr eval(Expr &expr, Env &env) {
    return expr.eval(env);
}
