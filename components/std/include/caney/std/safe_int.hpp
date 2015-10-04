/** @file */

#pragma once

#include "compiler_features.hpp"
#include "internal.hpp"

#include <limits>
#include <stdexcept>

// boost::numeric_cast
#include <boost/numeric/conversion/cast.hpp>

__CANEY_STDV1_BEGIN

namespace impl {
	template<typename Int>
	struct checked_int_ops {
		static_assert(std::numeric_limits<Int>::is_integer, "only works for integers");

		static constexpr Int min() noexcept {
			return std::numeric_limits<Int>::min();
		}

		static constexpr Int max() noexcept {
			return std::numeric_limits<Int>::max();
		}

		template<typename X = Int, typename std::enable_if<std::numeric_limits<X>::is_signed>::type* = nullptr>
		static CANEY_RELAXED_CONSTEXPR Int checked_add(Int const a, Int const b) {
			using std::to_string;
			if (a >= 0) {
				/* HAVE: a+b >= b >= min(), 0 <= (max() - a) <= max(), CHECK: a + b <= max() */
				if (b > max() - a) throw std::overflow_error(std::string(__func__) + ": addition positive overflow: " + to_string(a) + " + " + to_string(b));
			} else {
				/* HAVE: a+b < b <= max(), min() < (min() - a) <= 0, CHECK: a + b >= min() */
				if (b < min() - a) throw std::overflow_error(std::string(__func__) + ": addition negative overflow: " + to_string(a) + " + " + to_string(b));
			}
			return a + b;
		}

		template<typename X = Int, typename std::enable_if<!std::numeric_limits<X>::is_signed>::type* = nullptr>
		static CANEY_RELAXED_CONSTEXPR Int checked_add(Int const a, Int const b) {
			using std::to_string;
			/* unsigned variant: always a >= 0: */
			/* HAVE: a+b >= b >= min(), 0 <= (max() - a) <= max(), CHECK: a + b <= max() */
			if (b > max() - a) throw std::overflow_error(std::string(__func__) + ": addition positive overflow: " + to_string(a) + " + " + to_string(b));
			return a + b;
		}

		template<typename X = Int, typename std::enable_if<std::numeric_limits<X>::is_signed>::type* = nullptr>
		static CANEY_RELAXED_CONSTEXPR Int checked_sub(Int const a, Int const b) {
			using std::to_string;
			if (b < 0) {
				if (a > max() + b) throw std::overflow_error(std::string(__func__) + ": subtraction positive overflow: " + to_string(a) + " - " + to_string(b));
			} else {
				if (a < min() + b) throw std::overflow_error(std::string(__func__) + ": subtraction negative overflow: " + to_string(a) + " - " + to_string(b));
			}
			return a - b;
		}

		template<typename X = Int, typename std::enable_if<!std::numeric_limits<X>::is_signed>::type* = nullptr>
		static CANEY_RELAXED_CONSTEXPR Int checked_sub(Int const a, Int const b) {
			using std::to_string;
			/* unsigned variant: always b >= 0: */
			if (a < min() + b) throw std::overflow_error(std::string(__func__) + ": subtraction negative overflow: " + to_string(a) + " - " + to_string(b));
			return a - b;
		}

		template<typename X = Int, typename std::enable_if<std::numeric_limits<X>::is_signed>::type* = nullptr>
		static CANEY_RELAXED_CONSTEXPR Int checked_mul(Int const a, Int const b) {
			using std::to_string;
			if (0 == a || 0 == b) return a*b;
			if (a < 0) {
				if (b > 0) {
					if (a < min() / b) throw std::overflow_error(std::string(__func__) + ": multiplication negative overflow: " + to_string(a) + " * " + to_string(b));
				} else if (min() == a || min() == b) {
					throw std::overflow_error(std::string(__func__) + ": multiplication positive overflow: " + to_string(a) + " * " + to_string(b));
				} else /* a < 0, b < 0, (-a) and (-b) valid */ {
					if ((-a) > max() / (-b)) throw std::overflow_error(std::string(__func__) + ": multiplication positive overflow: " + to_string(a) + " * " + to_string(b));
				}
			} else { /* a > 0 */
				if (b < 0) {
					if (b < min() / a) throw std::overflow_error(std::string(__func__) + ": multiplication negative overflow: " + to_string(a) + " * " + to_string(b));
				} else /* b > 0 */ {
					if (a > max() / b) throw std::overflow_error(std::string(__func__) + ": multiplication positive overflow: " + to_string(a) + " * " + to_string(b));
				}
			}
			return a * b;
		}

		template<typename X = Int, typename std::enable_if<!std::numeric_limits<X>::is_signed>::type* = nullptr>
		static CANEY_RELAXED_CONSTEXPR Int checked_mul(Int const a, Int const b) {
			using std::to_string;
			if (0 == a || 0 == b) return a*b;
			/* unsigned variant: always a > 0 && b > 0: */
			if (a > max() / b) throw std::overflow_error(std::string(__func__) + ": multiplication positive overflow: " + to_string(a) + " * " + to_string(b));
			return a * b;
		}

		template<typename X = Int, typename std::enable_if<std::numeric_limits<X>::is_signed>::type* = nullptr>
		static CANEY_RELAXED_CONSTEXPR Int checked_div(Int const a, Int const b) {
			using std::to_string;
			if (a == min() && (b < 0 && b == Int{-1})) std::overflow_error(std::string(__func__) + ": division negative overflow: " + to_string(a) + " / " + to_string(b));
			if (0 == b) throw std::overflow_error(std::string(__func__) + ": division by zero: " + to_string(a) + " / " + to_string(b));
			return a / b;
		}

		template<typename X = Int, typename std::enable_if<!std::numeric_limits<X>::is_signed>::type* = nullptr>
		static CANEY_RELAXED_CONSTEXPR Int checked_div(Int const a, Int const b) {
			using std::to_string;
			/* unsigned variant: always b >= 0 */
			if (0 == b) throw std::overflow_error(std::string(__func__) + ": division by zero: " + to_string(a) + " / " + to_string(b));
			return a / b;
		}
	};
} // namespace impl

/**
 * @brief default `Tag` for @ref safe_int
 */
struct safe_int_default_tag;

/**
 * @brief wrap integer with overflow-checked primitive operations (+, -, *, /)
 *
 * Also a user can use different tags to create different types which are
 * not implicitly convertible.
 *
 * Example:
 * @code
 *     struct safe_int_userid_tag;
 *     using UserID = safe_int<uint32_t, safe_int_userid_tag>;
 *     struct safe_int_groupid_tag;
 *     using GroupID = safe_int<uint32_t, safe_int_groupid_tag>;
 *     // now UserID and GroupID are separate types
 * @endcode
 *
 * @tparam UnderylingInt underlying primitive integer type to wrap
 * @tparam Tag           some type to distinguish safe_int type instances
 */
template<typename UnderylingInt, typename Tag = safe_int_default_tag>
class safe_int {
private:
	using ops = impl::checked_int_ops<UnderylingInt>;

public:
	/// underlying primitive integer type
	using value_type = UnderylingInt;
	/// tag type
	using tag_type = Tag;

	/// default construct value to zero
	explicit constexpr safe_int() noexcept {
	}

	/**
	 * @brief initialize with given primitive value (requires explicit construction)
	 * @param value primitive value to initialize with
	 */
	explicit constexpr safe_int(value_type value) noexcept
	: m_value(value) {
	}

	/// @brief default copy constructor
	constexpr safe_int(safe_int const&) noexcept = default;
	/// @brief default copy assignment
	safe_int& operator=(safe_int const&) noexcept = default;

	/**
	 * @brief set primitive value
	 * @param value primitive value to set
	 */
	void set(value_type value) noexcept {
		m_value = value;
	}

	/**
	 * @brief access stored primitive value
	 * @return primitive value
	 */
	constexpr value_type get() const noexcept {
		return m_value;
	}

	/**
	 * @brief return minimum value the primitive type can store
	 * @return minimum value the primitive type can store
	 */
	static constexpr safe_int min() {
		return safe_int(ops::min());
	}

	/**
	 * @brief return maximum value the primitive type can store
	 * @return maximum value the primitive type can store
	 */
	static constexpr safe_int max() {
		return safe_int(ops::max());
	}

	/**
	 * @{
	 * @brief overflow checked primitive operation (throwing `std::overflow_error` on failures)
	 */
	friend safe_int& operator+=(safe_int& a, safe_int b) { return a = a + b; }
	friend safe_int& operator-=(safe_int& a, safe_int b) { return a = a - b; }
	friend safe_int& operator*=(safe_int& a, safe_int b) { return a = a * b; }
	friend safe_int& operator/=(safe_int& a, safe_int b) { return a = a / b; }

	friend constexpr safe_int operator+(safe_int a, safe_int b) { return safe_int(ops::checked_add(a.m_value, b.m_value)); }
	friend constexpr safe_int operator-(safe_int a)             { return safe_int(ops::checked_sub(value_type{0}, a.m_value)); }
	friend constexpr safe_int operator-(safe_int a, safe_int b) { return safe_int(ops::checked_sub(a.m_value, b.m_value)); }
	friend constexpr safe_int operator*(safe_int a, safe_int b) { return safe_int(ops::checked_mul(a.m_value, b.m_value)); }
	friend constexpr safe_int operator/(safe_int a, safe_int b) { return safe_int(ops::checked_div(a.m_value, b.m_value)); }
	/** @} */

	/**
	 * @{
	 * @brief default comparision operator on contained primitive type
	 */
	friend constexpr bool operator< (safe_int a, safe_int b) { return a.m_value <  b.m_value; }
	friend constexpr bool operator<=(safe_int a, safe_int b) { return a.m_value <= b.m_value; }
	friend constexpr bool operator> (safe_int a, safe_int b) { return a.m_value >  b.m_value; }
	friend constexpr bool operator>=(safe_int a, safe_int b) { return a.m_value >= b.m_value; }
	friend constexpr bool operator==(safe_int a, safe_int b) { return a.m_value == b.m_value; }
	friend constexpr bool operator!=(safe_int a, safe_int b) { return a.m_value != b.m_value; }
	/** @} */

private:
	value_type m_value = 0;
};

extern template class safe_int<char>;
extern template class safe_int<int8_t>;
extern template class safe_int<uint8_t>;
extern template class safe_int<int16_t>;
extern template class safe_int<uint16_t>;
extern template class safe_int<int32_t>;
extern template class safe_int<uint32_t>;
extern template class safe_int<int64_t>;
extern template class safe_int<uint64_t>;

__CANEY_STDV1_END
