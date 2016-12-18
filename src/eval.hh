/**
 * \file
 * \brief     Eval function.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2016, Chris Smeele
 * \license   TBD, do not redistribute.
 */
#pragma once

#include "common.hh"
#include "expression.hh"

/**
 * \brief Evaluate an expression.
 */
Eptr eval(Expr &expr, Env &env);
