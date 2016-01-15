/**
 * @file
 *
 * Similar to @ref caney::optional but uses a std::unique_ptr internally
 * (i.e. for large values which are often not present).
 */

#pragma once

#include "optional.hpp"

#include <memory>

__CANEY_STDV1_BEGIN

/**
 * A container either containing "nothing" or a value of type
 * `ValueType`
 *
 * See
 * http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2013/n3793.html
 *
 * @tparam ValueType type of possibly contained value
 */
template <typename ValueType>
class optional_ptr {
public:
	/** type of possibly contained value */
	using value_type = ValueType;
	static_assert(!std::is_same<nullopt_t, std::decay_t<value_type>>::value, "cannot contain nullopt_t");
	static_assert(!std::is_same<in_place_t, std::decay_t<value_type>>::value, "cannot contain in_place_t");

private:
	std::unique_ptr<ValueType> m_storage;

public:
	// X.Y.4.1 Constructors

	/**
	 * @brief initialize as disengaged
	 *
	 * @post `!*this`
	 */
	constexpr optional_ptr() noexcept {}

	/**
	 * @brief initialize as disengaged
	 *
	 * @post `!*this`
	 */
	/* implicit */ constexpr optional_ptr(nullopt_t const&) noexcept {}

	/**
	 * @brief If @p other is engaged copy value from `*other`, otherwise
	 *     initialize as disengaged
	 *
	 * @post `bool(*this) == bool(other)`
	 */
	optional_ptr(optional_ptr const& other) noexcept(std::is_nothrow_copy_constructible<value_type>::value) {
		if (other) { m_storage.reset(new value_type(*other)); }
	}

	/**
	 * @brief If @p other is engaged move storage, otherwise initialize as disengaged
	 */
	optional_ptr(optional_ptr&& other) noexcept {
		m_storage = std::move(other.m_storage);
	}

	/**
	 * @brief initialize as engaged by copying from given @p value
	 *
	 * @post `*this`
	 */
	optional_ptr(ValueType const& value) noexcept(std::is_nothrow_copy_constructible<value_type>::value) {
		m_storage.reset(new value_type(value));
	}

	/**
	 * @brief initialize as engaged by moving from given @p value
	 *
	 * @post `*this`
	 */
	optional_ptr(ValueType&& value) noexcept(std::is_nothrow_move_constructible<value_type>::value) {
		m_storage.reset(new value_type(std::move(value)));
	}

	/**
	 * @brief initialize as engaged by constructing from given @p args
	 *
	 * @post `*this`
	 */
	template <typename... Args>
	explicit optional_ptr(in_place_t, Args&&... args) {
		m_storage.reset(new value_type(std::forward<Args>(args)...));
	}

	/**
	 * @brief initialize as engaged by constructing from given
	 *     initializer list @p il and @p args
	 *
	 * @post `*this`
	 */
	template <typename ILValue, typename... Args>
	explicit optional_ptr(in_place_t, std::initializer_list<ILValue> il, Args&&... args) {
		m_storage.reset(new value_type(il, std::forward<Args>(args)...));
	}

	// X.Y.4.2 Destructor
	// implemented in base class, using a trivial
	// destructor if is_trivially_destructible<value_type>::value

	// X.Y.4.3, assignment

	/**
	 * @brief disengage
	 *
	 * @post `!*this`
	 */
	optional_ptr& operator=(nullopt_t const&) noexcept {
		m_storage = nullptr;
		return *this;
	}

	/**
	 * @brief if other is engaged `*this = *other`, otherwise `*this =
	 *     nullopt`
	 *
	 * @post `bool(*this) == bool(other)`
	 */
	optional_ptr&
	operator=(optional_ptr const& other) noexcept(std::is_nothrow_copy_constructible<value_type>::value&& std::is_nothrow_copy_assignable<value_type>::value) {
		if (other) {
			return * this = *other;
		} else {
			return * this = nullopt;
		}
	}

	/**
	 * @brief if other is engaged move storage, otherwise `*this = nullopt`
	 */
	optional_ptr&
	operator=(optional_ptr&& other) noexcept(std::is_nothrow_move_constructible<value_type>::value&& std::is_nothrow_move_assignable<value_type>::value) {
		if (other) {
			return * this = *std::move(other);
		} else {
			return * this = nullopt;
		}
	}

	/**
	 * @brief assign from value of (decayed) type @ref value_type
	 *     (prevent implicit conversions).
	 *
	 * @post `*this`
	 */
	template <typename Value, std::enable_if_t<std::is_same<value_type, std::decay_t<Value>>::value>* = nullptr>
	optional_ptr&
	operator=(Value&& value) noexcept(std::is_nothrow_constructible<value_type, Value&&>::value&& std::is_nothrow_assignable<value_type&, Value&&>::value) {
		if (m_storage) {
			*m_storage = std::forward<Value>(value);
		} else {
			m_storage.reset(new value_type(std::forward<Value>(value)));
		}
		return *this;
	}

	/**
	 * @brief emplace a newly constructed value
	 *
	 * @post `*this`
	 */
	template <typename... Args>
	void emplace(Args&&... args) {
		m_storage.reset(new value_type(std::forward<Args>(args)...));
	}

	/**
	 * @brief emplace a newly constructed value
	 *
	 * @post `*this`
	 */
	template <typename ILValue, typename... Args>
	void emplace(std::initializer_list<ILValue> il, Args&&... args) {
		m_storage.reset(new value_type(il, std::forward<Args>(args)...));
	}

	// X.Y.4.4 Swap

	/**
	 * @brief swap two values
	 */
	/* `noexcept(swap(std::declval<value_type&>(), std::declval<value_type&>()))` only works in std:: and std::-nested namespaces.
	 * using `noexcept(swap(std::declval<std::tuple<value_type>&>(), std::declval<std::tuple<value_type>&>()))` instead - tuples should have the right thing.
	 */
	void swap(optional_ptr& other) noexcept {
		m_storage.swap(other.m_storage);
	}

	// X.Y.4.5 Observers

	/**
	 * provide pointer-like access to inner value
	 *
	 * @pre `*this`
	 */
	CANEY_RELAXED_CONSTEXPR value_type* operator->() noexcept {
		return (assert(m_storage), m_storage.get());
	}

	/**
	 * provide pointer-like const access to inner value
	 *
	 * @pre `*this`
	 */
	constexpr value_type const* operator->() const noexcept {
		return (assert(m_storage), m_storage.get());
	}

	/**
	 * @brief get reference to value; `this` must be engaged
	 *
	 * @pre `*this`
	 */
	CANEY_RELAXED_CONSTEXPR value_type& operator*() & noexcept {
		return (assert(m_storage), *m_storage);
	}

	/**
	 * @brief get const reference to value; `this` must be engaged
	 *
	 * @pre `*this`
	 */
	constexpr value_type const& operator*() const& noexcept {
		return (assert(m_storage), *m_storage);
	}

	/**
	 * @brief get rvalue reference to value; `this` must be engaged
	 *
	 * @pre `*this`
	 */
	CANEY_RELAXED_CONSTEXPR value_type&& operator*() && noexcept {
		return (assert(m_storage), std::move(*m_storage));
	}

	/**
	 * whether `this` is engaged, i.e. not empty and contains a value
	 */
	constexpr explicit operator bool() const noexcept {
		return bool(m_storage);
	}

	/**
	 * @brief get reference to value; throws @ref bad_optional_access if
	 *     `this` is disengaged
	 *
	 * @throws bad_optional_access if `!*this`
	 */
	CANEY_RELAXED_CONSTEXPR value_type& value() & {
		return m_storage ? *m_storage : throw bad_optional_access("empty optional"), /* workaround gcc warning: */ *m_storage;
	}

	/**
	 * @brief get const reference to value; throws @ref
	 *     bad_optional_access if `this` is disengaged
	 *
	 * @throws bad_optional_access if `!*this`
	 */
	constexpr value_type const& value() const& {
		return m_storage ? *m_storage : throw bad_optional_access("empty optional"), *m_storage;
	}

	/**
	 * @brief get rvalue reference to value; throws @ref
	 *     bad_optional_access if `this` is disengaged
	 *
	 * @throws bad_optional_access if `!*this`
	 */
	CANEY_RELAXED_CONSTEXPR value_type&& value() && {
		return m_storage ? std::move(*m_storage) : throw bad_optional_access("empty optional"), std::move(*m_storage);
	}

	/**
	 * @brief get copy of value; returns @p def_value if `this` is
	 *     disengaged
	 */
	template <typename Value>
	constexpr value_type value_or(Value&& def_value) const& {
		return m_storage ? *m_storage : std::forward<Value>(def_value);
	}

	/**
	 * @brief extract value with move; returns @p def_value if `this` is
	 *     disengaged
	 */
	template <typename Value>
	CANEY_RELAXED_CONSTEXPR value_type value_or(Value&& def_value) && {
		return m_storage ? std::move(*m_storage) : std::forward<Value>(def_value);
	}
};

// X.Y.8, Relational operators
/**
 * @{
 * @brief relational operators on @ref optional (nullopt is less than
 *     any other value)
 */
template <typename ValueType>
constexpr bool operator==(optional_ptr<ValueType> const& a, optional_ptr<ValueType> const& b) {
	return a ? b && (*a == *b) : /* !a && */ !b;
}
template <typename ValueType>
constexpr bool operator!=(optional_ptr<ValueType> const& a, optional_ptr<ValueType> const& b) {
	return !(a == b);
}
template <typename ValueType>
constexpr bool operator<(optional_ptr<ValueType> const& a, optional_ptr<ValueType> const& b) {
	return b && (!a || (*a < *b));
}
template <typename ValueType>
constexpr bool operator>(optional_ptr<ValueType> const& a, optional_ptr<ValueType> const& b) {
	return (b < a);
}
template <typename ValueType>
constexpr bool operator<=(optional_ptr<ValueType> const& a, optional_ptr<ValueType> const& b) {
	return !(b < a);
}
template <typename ValueType>
constexpr bool operator>=(optional_ptr<ValueType> const& a, optional_ptr<ValueType> const& b) {
	return !(a < b);
}
/**
 * @}
 */

// X.Y.9, Comparison with nullopt
/**
 * @{
 * @brief relational operators on @ref optional_ptr and @ref nullopt_t
 *     (nullopt is less than any other value)
 */
template <typename ValueType>
constexpr bool operator==(optional_ptr<ValueType> const& a, nullopt_t) noexcept {
	return !a;
}
template <typename ValueType>
constexpr bool operator==(nullopt_t, optional_ptr<ValueType> const& a) noexcept {
	return !a;
}
template <typename ValueType>
constexpr bool operator!=(optional_ptr<ValueType> const& a, nullopt_t) noexcept {
	return bool(a);
}
template <typename ValueType>
constexpr bool operator!=(nullopt_t, optional_ptr<ValueType> const& a) noexcept {
	return bool(a);
}
template <typename ValueType>
constexpr bool operator<(optional_ptr<ValueType> const& a, nullopt_t) noexcept {
	return false;
}
template <typename ValueType>
constexpr bool operator<(nullopt_t, optional_ptr<ValueType> const& a) noexcept {
	return bool(a);
}
template <typename ValueType>
constexpr bool operator<=(optional_ptr<ValueType> const& a, nullopt_t) noexcept {
	return !a;
}
template <typename ValueType>
constexpr bool operator<=(nullopt_t, optional_ptr<ValueType> const& a) noexcept {
	return true;
}
template <typename ValueType>
constexpr bool operator>(optional_ptr<ValueType> const& a, nullopt_t) noexcept {
	return bool(a);
}
template <typename ValueType>
constexpr bool operator>(nullopt_t, optional_ptr<ValueType> const& a) noexcept {
	return false;
}
template <typename ValueType>
constexpr bool operator>=(optional_ptr<ValueType> const& a, nullopt_t) noexcept {
	return true;
}
template <typename ValueType>
constexpr bool operator>=(nullopt_t, optional_ptr<ValueType> const& a) noexcept {
	return !a;
}
/**
 * @}
 */

// X.Y.10, Comparison with T
/**
 * @{
 * @brief relational operators on @ref optional_ptr and @ref
 *     optional_ptr::value_type (nullopt is less than any other value)
 */
template <typename ValueType>
constexpr bool operator==(optional_ptr<ValueType> const& a, ValueType const& b) {
	return a && (*a == b);
}
template <typename ValueType>
constexpr bool operator==(ValueType const& a, optional_ptr<ValueType> const& b) {
	return b && (a == *b);
}
template <typename ValueType>
constexpr bool operator!=(optional_ptr<ValueType> const& a, ValueType const& b) {
	return !(a == b);
}
template <typename ValueType>
constexpr bool operator!=(ValueType const& a, optional_ptr<ValueType> const& b) {
	return !(a == b);
}
template <typename ValueType>
constexpr bool operator<(optional_ptr<ValueType> const& a, ValueType const& b) {
	return !a || (*a < b);
}
template <typename ValueType>
constexpr bool operator<(ValueType const& a, optional_ptr<ValueType> const& b) {
	return b && (a < *b);
}
template <typename ValueType>
constexpr bool operator<=(optional_ptr<ValueType> const& a, ValueType const& b) {
	return !(b > a);
}
template <typename ValueType>
constexpr bool operator<=(ValueType const& a, optional_ptr<ValueType> const& b) {
	return !(b > a);
}
template <typename ValueType>
constexpr bool operator>(optional_ptr<ValueType> const& a, ValueType const& b) {
	return !a && (*a > b);
}
template <typename ValueType>
constexpr bool operator>(ValueType const& a, optional_ptr<ValueType> const& b) {
	return b || (a > *b);
}
template <typename ValueType>
constexpr bool operator>=(optional_ptr<ValueType> const& a, ValueType const& b) {
	return !(a < b);
}
template <typename ValueType>
constexpr bool operator>=(ValueType const& a, optional_ptr<ValueType> const& b) {
	return !(a < b);
}
/**
 * @}
 */

// X.Y.11, Specialized algorithms
/**
 * @brief swap two values
 */
template <typename ValueType>
void swap(optional_ptr<ValueType>& a, optional_ptr<ValueType>& b) noexcept(noexcept(a.swap(b))) {
	a.swap(b);
}

/**
 * @brief create engaged optional_ptr from inner value
 */
template <typename Value>
constexpr optional_ptr<std::decay_t<Value>> make_optional_ptr(Value&& v) {
	return optional_ptr<std::decay_t<Value>>(in_place, std::forward<Value>(v));
}

__CANEY_STDV1_END

// X.Y.12, hash support
namespace std {
	template <class Key>
	struct hash;

	/**
	 * `std::hash` implementation for @ref caney::optional_ptr
	 */
	template <typename ValueType>
	struct hash<caney::optional_ptr<ValueType>> : private hash<ValueType> {
	private:
		using value_hash_t = hash<ValueType>;

	public:
		//! `std::hash<Key>::argument_type = Key`
		using argument_type = caney::optional_ptr<ValueType>;
		//! use result type from hash of inner value
		using result_type = typename value_hash_t::result_type;

		//! calculate hash from inner value or return default-constructed result
		result_type operator()(argument_type const& v) {
			return v ? value_hash_t::operator()(*v) : result_type{};
		}
	};
}
