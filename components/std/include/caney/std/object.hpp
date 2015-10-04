/** @file */

#pragma once

#include "internal.hpp"

__CANEY_STDV1_BEGIN

/**
 * base class to enforce virtual destructor; should be inherited
 * virtual; type shouldn't be used directly, so destructor is hidden.
 * copy and move probably doesn't make sense at all, require user to
 * override it if needed.
 */
class object {
protected:
	object() = default;
	virtual ~object() = default;

	/** @brief deleted @internal */
	object(const object&) = delete;
	/** @brief deleted @internal */
	object& operator=(const object&) = delete;
	/** @brief deleted @internal */
	object(object&&) = delete;
	/** @brief deleted @internal */
	object&& operator=(object&&) = delete;
};

__CANEY_STDV1_END
