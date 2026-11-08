/**
 * @file base.hpp
 *
 * The most basic core includes that mlpack expects; standard C++ includes and
 * Armadillo only
 *
 * mlpack is free software; you may redistribute it and/or modify it under the
 * terms of the 3-clause BSD license.  You should have received a copy of the
 * 3-clause BSD license along with mlpack.  If not, see
 * http://www.opensource.org/licenses/BSD-3-Clause for more information.
 */
#ifndef MLPACK_BASE_HPP
#define MLPACK_BASE_HPP

#if __cplusplus < 201703L
  #error "K-means requiert C++17 ou une version ultérieure"
#endif
#include <algorithm>
#include <cassert>
#include <cfloat>
#include <climits>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>
#include <memory>
#include <numeric>
#include <queue>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>
#include "config.hpp"
#include <armadillo>
#include <mlpack/core/util/arma_traits.hpp>

#if defined(__GNUG__) && !defined(DEBUG)
  #define mlpack_force_inline __attribute__((always_inline))
#else
  #define mlpack_force_inline
#endif

#ifdef _OPENMP
  #define MLPACK_USE_OPENMP
  #include <omp.h>
#endif

#endif
