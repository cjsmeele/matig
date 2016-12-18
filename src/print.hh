/**
 * \file
 * \brief     Print function.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2016, Chris Smeele
 * \license   TBD, do not redistribute.
 */
#pragma once

#include "common.hh"
#include "expression.hh"

#include <iostream>

/**
 * \brief Print an expression.
 */
void print(const Expr &expr, int depth = 0);
