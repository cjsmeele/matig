/**
 * \file
 * \brief     Eval function.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2016, Chris Smeele
 * \license   MIT, see LICENSE.
 */
#pragma once

#include "common.hh"
#include "expression.hh"

/**
 * \brief Evaluate an expression.
 */
Eptr eval(Expr &expr, Env &env);
