/**
 * @file
 *
 * Trying to implement:
 *   http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2013/n3793.html
 *
 * Requires at least some C++14 support.
 *
 * Ideas partially inspired from https://github.com/akrzemi1/Optional,
 * but the spec doesn't leave much room anyway :)
 */

#pragma once

#include "addressof.hpp"
#include "compiler_features.hpp"
#include "internal.hpp"
#include "tags.hpp"

#include <cassert>
#include <initializer_list>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <utility>

__CANEY_STDV1_BEGIN

/**
 * @brief explicit disengaged value type for @ref optional. construct by
 *     copying from @ref nullopt.
 */
struct nullopt_t {
	//! @cond INTERNAL
	enum class tag_t { tag };
	explicit constexpr nullopt_t(tag_t){};
	//! @endcond
};

/**
 * @brief explicit disengaged value of type @ref nullopt_t for @ref optional
 * @hideinitializer
 */
constexpr nullopt_t nullopt{nullopt_t::tag_t::tag};

/**
 * @brief exception thrown when attempting to access a disengaged @ref
 *     optional value through @ref optional::value()
 */
class bad_optional_access : public std::logic_error {
public:
	/** constructor */
	explicit bad_optional_access(const std::string& what_arg) : logic_error(what_arg) {}

	/** constructor */
	explicit bad_optional_access(const char* what_arg) : logic_error(what_arg) {}
};

namespace impl {
	/* union doesn't call constructors/destructors of contained members
	 * automatically; we use this to call them when we need to
	 */

	template <typename ValueType>
	class normal_optional_base {
	public:
		using value_type = ValueType;

	protected:
		union storage {
			nullopt_t m_nullopt; /* dummy member for constexpr constructor */
			ValueType m_value;

			explicit constexpr storage() noexcept : m_nullopt(nullopt) {}

			explicit constexpr storage(ValueType const& value) : m_value(value) {}

			explicit constexpr storage(ValueType&& value) : m_value(std::move(value)) {}

			template <typename... Args>
			explicit constexpr storage(in_place_t, Args&&... args) : m_value(std::forward<Args>(args)...) {}

			template <typename ILValue, typename... Args>
			explicit constexpr storage(in_place_t, std::initializer_list<ILValue> il, Args&&... args) : m_value(il, std::forward<Args>(args)...) {}

			~storage() noexcept {}
		};

	public:
		explicit constexpr normal_optional_base() noexcept = default;

		explicit constexpr normal_optional_base(ValueType const& value) noexcept(std::is_nothrow_copy_constructible<value_type>::value)
		: m_storage(value), m_valid(true) {}

		explicit constexpr normal_optional_base(ValueType&& value) noexcept(std::is_nothrow_move_constructible<value_type>::value)
		: m_storage(std::move(value)), m_valid(true) {}

		template <typename... Args>
		explicit constexpr normal_optional_base(in_place_t, Args&&... args) : m_storage(in_place, std::forward<Args>(args)...), m_valid(true) {}

		template <typename ILValue, typename... Args>
		explicit constexpr normal_optional_base(in_place_t, std::initializer_list<ILValue> il, Args&&... args)
		: m_storage(in_place, il, std::forward<Args>(args)...), m_valid(true) {}

		void clear() noexcept {
			if (m_valid) {
				m_storage.m_value.value_type::~value_type();
				m_valid = false;
			}
		}

	protected:
		~normal_optional_base() noexcept {
			clear();
		}

		storage m_storage;
		bool m_valid = false;
	};

	template <typename ValueType>
	class constexpr_optional_base {
	public:
		using value_type = ValueType;

	protected:
		union storage {
			nullopt_t m_nullopt; /* dummy member for constexpr constructor */
			ValueType m_value;

			explicit constexpr storage() noexcept : m_nullopt(nullopt) {}

			explicit constexpr storage(ValueType const& value) : m_value(value) {}

			explicit constexpr storage(ValueType&& value) : m_value(std::move(value)) {}

			template <typename... Args>
			explicit constexpr storage(in_place_t, Args&&... args) : m_value(std::forward<Args>(args)...) {}

			template <typename ILValue, typename... Args>
			explicit constexpr storage(in_place_t, std::initializer_list<ILValue> il, Args&&... args) : m_value(il, std::forward<Args>(args)...) {}

			~storage() noexcept = default;
		};

	public:
		explicit constexpr constexpr_optional_base() noexcept = default;

		explicit constexpr constexpr_optional_base(ValueType const& value) noexcept(std::is_nothrow_copy_constructible<value_type>::value)
		: m_storage(value), m_valid(true) {}

		explicit constexpr constexpr_optional_base(ValueType&& value) noexcept(std::is_nothrow_move_constructible<value_type>::value)
		: m_storage(std::move(value)), m_valid(true) {}

		template <typename... Args>
		explicit constexpr constexpr_optional_base(in_place_t, Args&&... args) : m_storage(in_place, std::forward<Args>(args)...), m_valid(true) {}

		template <typename ILValue, typename... Args>
		explicit constexpr constexpr_optional_base(in_place_t, std::initializer_list<ILValue> il, Args&&... args)
		: m_storage(in_place, il, std::forward<Args>(args)...), m_valid(true) {}

		void clear() noexcept {
			m_valid = false;
		}

	protected:
		storage m_storage;
		bool m_valid = false;
	};

	template <typename ValueType>
	using optional_base =
		std::conditional_t<std::is_trivially_destructible<ValueType>::value, constexpr_optional_base<ValueType>, normal_optional_base<ValueType>>;
}

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
class optional : protected impl::optional_base<ValueType> {
public:
	/** type of possibly contained value */
	using value_type = ValueType;
	static_assert(!std::is_same<nullopt_t, std::decay_t<value_type>>::value, "cannot contain nullopt_t");
	static_assert(!std::is_same<in_place_t, std::decay_t<value_type>>::value, "cannot contain in_place_t");

private:
	using base = impl::optional_base<value_type>;
	using base::m_storage;
	using base::m_valid;

public:
	// X.Y.4.1 Constructors

	/**
	 * @brief initialize as disengaged
	 *
	 * @post `!*this`
	 */
	constexpr optional() noexcept : base() {}

	/**
	 * @brief initialize as disengaged
	 *
	 * @post `!*this`
	 */
	/* implicit */ constexpr optional(nullopt_t const&) noexcept : base() {}

	/**
	 * @brief If @p other is engaged copy value from `*other`, otherwise
	 *     initialize as disengaged
	 *
	 * @post `bool(*this) == bool(other)`
	 */
	CANEY_RELAXED_CONSTEXPR optional(optional const& other) noexcept(std::is_nothrow_copy_constructible<value_type>::value) : base() {
		if (other) {
			::new (static_cast<void*>(address())) value_type(*other);
			m_valid = true;
		}
	}

	/**
	 * @brief If @p other is engaged copy value from
	 *     `*std::``move(other)`, otherwise initialize as disengaged
	 *
	 * @post `bool(*this) == bool(other)`
	 */
	CANEY_RELAXED_CONSTEXPR optional(optional&& other) noexcept(std::is_nothrow_move_constructible<value_type>::value) : base() {
		if (other) {
			::new (static_cast<void*>(address())) value_type(*std::move(other));
			m_valid = true;
		}
	}

	/**
	 * @brief initialize as engaged by copying from given @p value
	 *
	 * @post `*this`
	 */
	constexpr optional(ValueType const& value) noexcept(std::is_nothrow_copy_constructible<value_type>::value) : base(value) {}

	/**
	 * @brief initialize as engaged by moving from given @p value
	 *
	 * @post `*this`
	 */
	constexpr optional(ValueType&& value) noexcept(std::is_nothrow_move_constructible<value_type>::value) : base(std::move(value)) {}

	/**
	 * @brief initialize as engaged by constructing from given @p args
	 *
	 * @post `*this`
	 */
	template <typename... Args>
	explicit constexpr optional(in_place_t, Args&&... args) : base(in_place, std::forward<Args>(args)...) {}

	/**
	 * @brief initialize as engaged by constructing from given
	 *     initializer list @p il and @p args
	 *
	 * @post `*this`
	 */
	template <typename ILValue, typename... Args>
	explicit constexpr optional(in_place_t, std::initializer_list<ILValue> il, Args&&... args) : base(in_place, il, std::forward<Args>(args)...) {}

	// X.Y.4.2 Destructor
	// implemented in base class, using a trivial
	// destructor if is_trivially_destructible<value_type>::value

	// X.Y.4.3, assignment

	/**
	 * @brief disengage
	 *
	 * @post `!*this`
	 */
	optional& operator=(nullopt_t const&) noexcept {
		base::clear();
		return *this;
	}

	/**
	 * @brief if other is engaged `*this = *other`, otherwise `*this =
	 *     nullopt`
	 *
	 * @post `bool(*this) == bool(other)`
	 */
	optional&
	operator=(optional const& other) noexcept(std::is_nothrow_copy_constructible<value_type>::value&& std::is_nothrow_copy_assignable<value_type>::value) {
		if (other) {
			return * this = *other;
		} else {
			return * this = nullopt;
		}
	}

	/**
	 * @brief if other is engaged `*this = *std::move(other)`, otherwise
	 *     `*this = nullopt`
	 *
	 * @post `bool(*this) == bool(other)`
	 */
	optional& operator=(optional&& other) noexcept(std::is_nothrow_move_constructible<value_type>::value&& std::is_nothrow_move_assignable<value_type>::value) {
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
	optional&
	operator=(Value&& value) noexcept(std::is_nothrow_constructible<value_type, Value&&>::value&& std::is_nothrow_assignable<value_type&, Value&&>::value) {
		if (m_valid) {
			m_storage.m_value = std::forward<Value>(value);
		} else {
			::new (static_cast<void*>(address())) value_type(std::forward<Value>(value));
			m_valid = true;
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
		*this = nullopt;
		::new (static_cast<void*>(address())) value_type(std::forward<Args>(args)...);
		m_valid = true;
	}

	/**
	 * @brief emplace a newly constructed value
	 *
	 * @post `*this`
	 */
	template <typename ILValue, typename... Args>
	void emplace(std::initializer_list<ILValue> il, Args&&... args) {
		*this = nullopt;
		::new (static_cast<void*>(address())) value_type(il, std::forward<Args>(args)...);
		m_valid = true;
	}

	// X.Y.4.4 Swap

	/**
	 * @brief swap two values
	 */
	/* `noexcept(swap(std::declval<value_type&>(), std::declval<value_type&>()))` only works in std:: and std::-nested namespaces.
	 * using `noexcept(swap(std::declval<std::tuple<value_type>&>(), std::declval<std::tuple<value_type>&>()))` instead - tuples should have the right thing.
	 */
	void swap(optional& other) noexcept(
		std::is_nothrow_move_constructible<value_type>::value&& noexcept(
			swap(std::declval<std::tuple<value_type>&>(), std::declval<std::tuple<value_type>&>()))) {
		if (*this) {
			if (other) {
				using std::swap;
				swap(**this, *other);
			} else {
				other = std::move(**this);
				base::clear();
			}
		} else if (other) {
			*this = std::move(*other);
			other.base::clear();
		}
	}

	// X.Y.4.5 Observers

	/**
	 * provide pointer-like access to inner value
	 *
	 * @pre `*this`
	 */
	CANEY_RELAXED_CONSTEXPR value_type* operator->() noexcept {
		return (assert(m_valid), address());
	}

	/**
	 * provide pointer-like const access to inner value
	 *
	 * @pre `*this`
	 */
	constexpr value_type const* operator->() const noexcept {
		return (assert(m_valid), address());
	}

	/**
	 * @brief get reference to value; `this` must be engaged
	 *
	 * @pre `*this`
	 */
	CANEY_RELAXED_CONSTEXPR value_type& operator*() & noexcept {
		return (assert(m_valid), m_storage.m_value);
	}

	/**
	 * @brief get const reference to value; `this` must be engaged
	 *
	 * @pre `*this`
	 */
	constexpr value_type const& operator*() const& noexcept {
		return (assert(m_valid), m_storage.m_value);
	}

	/**
	 * @brief get rvalue reference to value; `this` must be engaged
	 *
	 * @pre `*this`
	 */
	CANEY_RELAXED_CONSTEXPR value_type&& operator*() && noexcept {
		return (assert(m_valid), std::move(m_storage.m_value));
	}

	/**
	 * whether `this` is engaged, i.e. not empty and contains a value
	 */
	constexpr explicit operator bool() const noexcept {
		return m_valid;
	}

	/**
	 * @brief get reference to value; throws @ref bad_optional_access if
	 *     `this` is disengaged
	 *
	 * @throws bad_optional_access if `!*this`
	 */
	CANEY_RELAXED_CONSTEXPR value_type& value() & {
		return m_valid ? m_storage.m_value : throw bad_optional_access("empty optional"), /* workaround gcc warning: */ m_storage.m_value;
	}

	/**
	 * @brief get const reference to value; throws @ref
	 *     bad_optional_access if `this` is disengaged
	 *
	 * @throws bad_optional_access if `!*this`
	 */
	constexpr value_type const& value() const& {
		return m_valid ? m_storage.m_value : throw bad_optional_access("empty optional"), m_storage.m_value;
	}

	/**
	 * @brief get rvalue reference to value; throws @ref
	 *     bad_optional_access if `this` is disengaged
	 *
	 * @throws bad_optional_access if `!*this`
	 */
	CANEY_RELAXED_CONSTEXPR value_type&& value() && {
		return m_valid ? std::move(m_storage.m_value) : throw bad_optional_access("empty optional"), std::move(m_storage.m_value);
	}

	/**
	 * @brief get copy of value; returns @p def_value if `this` is
	 *     disengaged
	 */
	template <typename Value>
	constexpr value_type value_or(Value&& def_value) const& {
		return m_valid ? m_storage.m_value : std::forward<Value>(def_value);
	}

	/**
	 * @brief extract value with move; returns @p def_value if `this` is
	 *     disengaged
	 */
	template <typename Value>
	CANEY_RELAXED_CONSTEXPR value_type value_or(Value&& def_value) && {
		return m_valid ? std::move(m_storage.m_value) : std::forward<Value>(def_value);
	}

private:
	CANEY_RELAXED_CONSTEXPR value_type* address() {
		return caney::addressof(m_storage.m_value);
	}

	constexpr value_type const* address() const {
		return caney::addressof(m_storage.m_value);
	}
};

// X.Y.8, Relational operators
/**
 * @{
 * @brief relational operators on @ref optional (nullopt is less than
 *     any other value)
 */
template <typename ValueType>
constexpr bool operator==(optional<ValueType> const& a, optional<ValueType> const& b) {
	return a ? b && (*a == *b) : /* !a && */ !b;
}
template <typename ValueType>
constexpr bool operator!=(optional<ValueType> const& a, optional<ValueType> const& b) {
	return !(a == b);
}
template <typename ValueType>
constexpr bool operator<(optional<ValueType> const& a, optional<ValueType> const& b) {
	return b && (!a || (*a < *b));
}
template <typename ValueType>
constexpr bool operator>(optional<ValueType> const& a, optional<ValueType> const& b) {
	return (b < a);
}
template <typename ValueType>
constexpr bool operator<=(optional<ValueType> const& a, optional<ValueType> const& b) {
	return !(b < a);
}
template <typename ValueType>
constexpr bool operator>=(optional<ValueType> const& a, optional<ValueType> const& b) {
	return !(a < b);
}
/**
 * @}
 */

// X.Y.9, Comparison with nullopt
/**
 * @{
 * @brief relational operators on @ref optional and @ref nullopt_t
 *     (nullopt is less than any other value)
 */
template <typename ValueType>
constexpr bool operator==(optional<ValueType> const& a, nullopt_t) noexcept {
	return !a;
}
template <typename ValueType>
constexpr bool operator==(nullopt_t, optional<ValueType> const& a) noexcept {
	return !a;
}
template <typename ValueType>
constexpr bool operator!=(optional<ValueType> const& a, nullopt_t) noexcept {
	return bool(a);
}
template <typename ValueType>
constexpr bool operator!=(nullopt_t, optional<ValueType> const& a) noexcept {
	return bool(a);
}
template <typename ValueType>
constexpr bool operator<(optional<ValueType> const& a, nullopt_t) noexcept {
	return false;
}
template <typename ValueType>
constexpr bool operator<(nullopt_t, optional<ValueType> const& a) noexcept {
	return bool(a);
}
template <typename ValueType>
constexpr bool operator<=(optional<ValueType> const& a, nullopt_t) noexcept {
	return !a;
}
template <typename ValueType>
constexpr bool operator<=(nullopt_t, optional<ValueType> const& a) noexcept {
	return true;
}
template <typename ValueType>
constexpr bool operator>(optional<ValueType> const& a, nullopt_t) noexcept {
	return bool(a);
}
template <typename ValueType>
constexpr bool operator>(nullopt_t, optional<ValueType> const& a) noexcept {
	return false;
}
template <typename ValueType>
constexpr bool operator>=(optional<ValueType> const& a, nullopt_t) noexcept {
	return true;
}
template <typename ValueType>
constexpr bool operator>=(nullopt_t, optional<ValueType> const& a) noexcept {
	return !a;
}
/**
 * @}
 */

// X.Y.10, Comparison with T
/**
 * @{
 * @brief relational operators on @ref optional and @ref
 *     optional::value_type (nullopt is less than any other value)
 */
template <typename ValueType>
constexpr bool operator==(optional<ValueType> const& a, ValueType const& b) {
	return a && (*a == b);
}
template <typename ValueType>
constexpr bool operator==(ValueType const& a, optional<ValueType> const& b) {
	return b && (a == *b);
}
template <typename ValueType>
constexpr bool operator!=(optional<ValueType> const& a, ValueType const& b) {
	return !(a == b);
}
template <typename ValueType>
constexpr bool operator!=(ValueType const& a, optional<ValueType> const& b) {
	return !(a == b);
}
template <typename ValueType>
constexpr bool operator<(optional<ValueType> const& a, ValueType const& b) {
	return !a || (*a < b);
}
template <typename ValueType>
constexpr bool operator<(ValueType const& a, optional<ValueType> const& b) {
	return b && (a < *b);
}
template <typename ValueType>
constexpr bool operator<=(optional<ValueType> const& a, ValueType const& b) {
	return !(b > a);
}
template <typename ValueType>
constexpr bool operator<=(ValueType const& a, optional<ValueType> const& b) {
	return !(b > a);
}
template <typename ValueType>
constexpr bool operator>(optional<ValueType> const& a, ValueType const& b) {
	return !a && (*a > b);
}
template <typename ValueType>
constexpr bool operator>(ValueType const& a, optional<ValueType> const& b) {
	return b || (a > *b);
}
template <typename ValueType>
constexpr bool operator>=(optional<ValueType> const& a, ValueType const& b) {
	return !(a < b);
}
template <typename ValueType>
constexpr bool operator>=(ValueType const& a, optional<ValueType> const& b) {
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
void swap(optional<ValueType>& a, optional<ValueType>& b) noexcept(noexcept(a.swap(b))) {
	a.swap(b);
}

/**
 * @brief create engaged optional from inner value
 */
template <typename Value>
constexpr optional<std::decay_t<Value>> make_optional(Value&& v) {
	return optional<std::decay_t<Value>>(in_place, std::forward<Value>(v));
}

__CANEY_STDV1_END

// X.Y.12, hash support
namespace std {
	template <class Key>
	struct hash;

	/**
	 * `std::hash` implementation for @ref caney::optional
	 */
	template <typename ValueType>
	struct hash<caney::optional<ValueType>> : private hash<ValueType> {
	private:
		using value_hash_t = hash<ValueType>;

	public:
		//! `std::hash<Key>::argument_type = Key`
		using argument_type = caney::optional<ValueType>;
		//! use result type from hash of inner value
		using result_type = typename value_hash_t::result_type;

		//! calculate hash from inner value or return default-constructed result
		result_type operator()(argument_type const& v) {
			return v ? value_hash_t::operator()(*v) : result_type{};
		}
	};
}
