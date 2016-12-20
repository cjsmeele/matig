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
 *
 * \param expr The expression to evaluate
 * \param env  The environment in which the expression will be evaluated
 *
 * \return The evaluation result
 */
Eptr eval(Expr &expr, Env &env);
