/** @file */

#pragma once

#include "internal.hpp"

#include <utility>

__CANEY_UTILV1_BEGIN

/**
 * use move semantic on wrapped value in copy-constructor
 *
 * use as wrapper for argument types in function<...> to enforce moving
 * when calling or binding it as parameter.
 *
 * calling twice after binding such argument will lead to unexpected results.
 *
 * use wrap_dispatch/wrap_post from dispatcher.hpp instead if possible.
 */
template<typename T>
class move_arg {
public:
	/** wrapped type */
	using value_type = T;

	/**
	 * @brief implicit construction from wrapped value
	 */
	move_arg(value_type&& value)
	: m_value(std::move(value)) {
	}

	/**
	 * @brief move instead of copy!
	 * @param other wrapped value to move from
	 */
	move_arg(move_arg const& other)
	: m_value(std::move(other.m_value)) {
	}

	/**
	 * @brief deleted, copy assignment not allowed
	 * @internal
	 * @return *this
	 */
	move_arg& operator=(move_arg const&) = delete;

	/**
	 * @brief default move construction
	 */
	move_arg(move_arg&& other) = default;

	/**
	 * @brief default move assignment
	 * @return *this
	 */
	move_arg& operator=(move_arg&&) = default;

	/**
	 * @brief implicit conversion to wrapped type (moves wrapped value out);
	 * alias for @ref extract()
	 */
	operator value_type() const {
		return std::move(m_value);
	}

	/**
	 * @brief set new wrapped value
	 * @param value value to set
	 */
	void set(value_type&& value) const {
		m_value = std::move(value);
	}

	/**
	 * @brief get normal reference to stored value (doesn't move anything)
	 * @return reference to wrapped value
	 */
	value_type& get() const {
		return m_value;
	}

	/**
	 * @brief extract wrapped value (moves wrapped value out)
	 */
	value_type extract() const {
		return std::move(m_value);
	}

private:
	mutable value_type m_value;
};

__CANEY_UTILV1_END
