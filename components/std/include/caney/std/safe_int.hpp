#pragma once

#include "compiler_features.hpp"

#include <limits>
#include <stdexcept>

// boost::numeric_cast
#include <boost/numeric/conversion/cast.hpp>

namespace caney {
	inline namespace stdv1 {
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

		struct safe_int_default_tag;

		// tag can be used to create different types
		template<typename UnderylingInt, typename Tag = safe_int_default_tag>
		class safe_int {
		private:
			using ops = impl::checked_int_ops<UnderylingInt>;

		public:
			using value_type = UnderylingInt;
			using tag_type = Tag;

			explicit constexpr safe_int() noexcept {
			}

			explicit constexpr safe_int(value_type value) noexcept
			: m_value(value) {
			}

			constexpr safe_int(safe_int const& other) noexcept = default;
			safe_int& operator=(safe_int const& other) noexcept = default;

			void set(value_type value) noexcept {
				m_value = value;
			}

			constexpr value_type get() const noexcept {
				return m_value;
			}

			static constexpr safe_int min() {
				return safe_int(ops::min());
			}

			static constexpr safe_int max() {
				return safe_int(ops::max());
			}

			friend safe_int& operator+=(safe_int& a, safe_int b) { return a = a + b; }
			friend safe_int& operator-=(safe_int& a, safe_int b) { return a = a - b; }
			friend safe_int& operator*=(safe_int& a, safe_int b) { return a = a * b; }
			friend safe_int& operator/=(safe_int& a, safe_int b) { return a = a / b; }

			friend constexpr safe_int operator+(safe_int a, safe_int b) { return safe_int(ops::checked_add(a.m_value, b.m_value)); }
			friend constexpr safe_int operator-(safe_int a)             { return safe_int(ops::checked_sub(value_type{0}, a.m_value)); }
			friend constexpr safe_int operator-(safe_int a, safe_int b) { return safe_int(ops::checked_sub(a.m_value, b.m_value)); }
			friend constexpr safe_int operator*(safe_int a, safe_int b) { return safe_int(ops::checked_mul(a.m_value, b.m_value)); }
			friend constexpr safe_int operator/(safe_int a, safe_int b) { return safe_int(ops::checked_div(a.m_value, b.m_value)); }

			friend constexpr bool operator< (safe_int a, safe_int b) { return a.m_value <  b.m_value; }
			friend constexpr bool operator<=(safe_int a, safe_int b) { return a.m_value <= b.m_value; }
			friend constexpr bool operator> (safe_int a, safe_int b) { return a.m_value >  b.m_value; }
			friend constexpr bool operator>=(safe_int a, safe_int b) { return a.m_value >= b.m_value; }
			friend constexpr bool operator==(safe_int a, safe_int b) { return a.m_value == b.m_value; }
			friend constexpr bool operator!=(safe_int a, safe_int b) { return a.m_value != b.m_value; }

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
	}
} // namespace caney
