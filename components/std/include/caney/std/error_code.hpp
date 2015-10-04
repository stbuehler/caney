/** @file */

#pragma once

#include "internal.hpp"

#include <system_error>

__CANEY_STDV1_BEGIN

/**
 * @brief create `std::error_code` from `errno`
 * @return `std::error_code` for errno in `std::generic_category()`
 */
std::error_code errno_error_code();

__CANEY_STDV1_END
