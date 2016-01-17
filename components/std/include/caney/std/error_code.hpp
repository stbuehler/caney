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

/**
 * @brief create `std::error_code` for some user-specified `errno`-compatible error
 * @return `std::error_code` for user_errno in `std::generic_category()`
 */
std::error_code errno_error_code(int user_errno);

__CANEY_STDV1_END
