/** @file */

#pragma once

#include "internal.hpp"

__CANEY_DOXYGEN_GROUP_STDV1_BEGIN

/**
 * @brief `constexpr` if C++-14 relaxed `constexpr` support is active, otherwise empty
 */
#if __cpp_constexpr >= 201304
#define CANEY_RELAXED_CONSTEXPR constexpr
#else
#define CANEY_RELAXED_CONSTEXPR
#endif

__CANEY_DOXYGEN_GROUP_STDV1_END
