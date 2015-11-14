/** @file */

#pragma once

#include "internal.hpp"

__CANEY_STDV1_BEGIN

/**
 * @brief constexpr version of the std::addressof() function
 */
template<typename T>
constexpr T* addressof(T& v) {
	return reinterpret_cast<T*>(
		&const_cast<char&>(
			reinterpret_cast<const volatile char&>(v)));
}

__CANEY_STDV1_END
