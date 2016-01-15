/** @file */

#pragma once

#include "internal.hpp"

__CANEY_DOXYGEN_GROUP_UTILV1_BEGIN

/**
  * mark an expression as being likely evaluated to 1 (true)
  * @param x (boolean) expression
  */
#ifdef __builtin_expect
#define CANEY_LIKELY(x) __builtin_expect((x), 1)
#else
#define CANEY_LIKELY(x) (x)
#endif

/**
  * mark an expression as being likely evaluated to 0 (false)
  * @param x (boolean) expression
  */
#ifdef __builtin_expect
#define CANEY_UNLIKELY(x) __builtin_expect((x), 0)
#else
#define CANEY_UNLIKELY(x) (x)
#endif

__CANEY_DOXYGEN_GROUP_UTILV1_END
