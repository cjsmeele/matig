#pragma once

#include "common.hh"
#include "expression.hh"

/**
 * \brief Evaluate an expression.
 */
Eptr eval(Expr &expr, Env &env);
