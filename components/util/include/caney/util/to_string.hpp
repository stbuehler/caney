/** @file */

#pragma once

#include "internal.hpp"

#include <cstdint>
#include <limits>
#include <string>
#include <type_traits>

__CANEY_UTILV1_BEGIN

namespace impl {
	std::string to_string(uintmax_t val);
	std::string to_string(intmax_t val);
} // namespace impl

/**
 * @brief locale-independent (signed) integer to string conversion
 * @param val
 * @return decimal representation of signed integer (only digits + sign if negative)
 */
template <typename Integral, std::enable_if_t<std::numeric_limits<Integral>::is_integer && std::numeric_limits<Integral>::is_signed>* = nullptr>
std::string to_string(Integral val) {
	return impl::to_string(intmax_t{val});
}

/**
 * @brief locale-independent (unsigned) integer to string conversion
 * @param val
 * @return decimal representation of unsigned integer (only digits)
 */
template <typename Integral, std::enable_if_t<std::numeric_limits<Integral>::is_integer && !std::numeric_limits<Integral>::is_signed>* = nullptr>
std::string to_string(Integral val) {
	return impl::to_string(uintmax_t{val});
}

__CANEY_UTILV1_END
