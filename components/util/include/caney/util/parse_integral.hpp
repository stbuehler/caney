/** @file */

#pragma once

#include "caney/memory/buffer.hpp"
#include "caney/std/optional.hpp"

#include "internal.hpp"
#include "macros.hpp"

#include <type_traits>

__CANEY_UTILV1_BEGIN

#if defined(DOXYGEN)
	/**
	 * @brief parse number from minimal decimal string representation
	 *
	 * - optional '-' prefix for negative values if `Integral` is signed type
	 * - no leading zeroes permitted
	 * - no leading '+' permitted
	 * - returns remaining data (beginning at first invalid character) in `str` parameter
	 * - returns caney::nullopt for any error (invalid data, overflow); doesn't modify `str` in this case
	 *
	 * @tparam Integral integer type (signed or unsigned)
	 * @param str string to parse
	 */
	template<typename Integral>
	caney::optional<Integral> parse_integral_open(memory::raw_const_buf& str);
#else // defined(DOXYGEN)
	template<typename Integral, std::enable_if_t<std::is_signed<Integral>::value>* = nullptr>
	caney::optional<Integral> parse_integral_open(memory::raw_const_buf& str) {
		unsigned char constexpr digit_0{'0'};
		Integral constexpr Min = std::numeric_limits<Integral>::min();
		Integral constexpr Max = std::numeric_limits<Integral>::max();
		memory::raw_const_buf strCopy{str};

		if (strCopy.empty()) return caney::nullopt;
		if ('-' == strCopy[size_t{0}]) {
			strCopy = strCopy.raw_slice(1);
			if (strCopy.empty()) return caney::nullopt;
			Integral result = 0;
			for (auto const &c: strCopy) {
				unsigned char const digit = c - digit_0;
				if (CANEY_UNLIKELY(digit > 9)) {
					if (&c == strCopy.begin()) return caney::nullopt;
					str = strCopy.raw_slice(static_cast<std::size_t>(&c - strCopy.begin()));
					return result;
				}
				Integral const digitValue = static_cast<Integral>(digit);
				/*    (-(Min + 10)) % 10 is the last digit of Min */
				if (CANEY_UNLIKELY(result <= Min / 10) && (result < Min / 10 || digitValue > (-(Min + 10)) % 10)) return caney::nullopt;
				result = 10 * result - digitValue;
			}
			str.reset();
			return result;
		} else {
			Integral result = 0;
			for (auto c: strCopy) {
				unsigned char const digit = c - digit_0;
				if (CANEY_UNLIKELY(digit > 9)) {
					if (&c == strCopy.begin()) return caney::nullopt;
					str = strCopy.raw_slice(static_cast<std::size_t>(&c - strCopy.begin()));
					return result;
				}
				Integral const digitValue = static_cast<Integral>(digit);
				if (CANEY_UNLIKELY(result >= Max / 10) && (result > Max / 10 || digitValue > Max % 10)) return caney::nullopt;
				result = 10 * result + digit;
			}
			str.reset();
			return result;
		}
	}

	template<typename Integral, std::enable_if_t<std::is_unsigned<Integral>::value>* = nullptr>
	caney::optional<Integral> parse_integral_open(memory::raw_const_buf& str) {
		unsigned char constexpr digit_0{'0'};
		Integral constexpr Max = std::numeric_limits<Integral>::max();

		if (str.empty()) return caney::nullopt;
		Integral result = 0;
		for (auto c: str) {
			unsigned char const digit = c - digit_0;
			if (CANEY_UNLIKELY(digit > 9)) {
				if (&c == str.begin()) return caney::nullopt;
				str = str.raw_slice(static_cast<std::size_t>(&c - str.begin()));
				return result;
			}
			Integral const digitValue = static_cast<Integral>(digit);
			if (CANEY_UNLIKELY(result >= Max / 10) && (result > Max / 10 || digitValue > Max % 10)) return caney::nullopt;
			result = 10 * result + digit;
		}
		str.reset();
		return result;
	}
#endif // else (defined(DOXYGEN))

/**
 * @brief parse_integral<Integral>(str): parse number from minimal decimal string representation
 *
 * - optional '-' prefix for negative values if `Integral` is signed type
 * - no leading zeroes permitted
 * - no leading '+' permitted
 * - only valid if input consists only of the number (no whitespace allowed)
 * - returns caney::nullopt for any error (invalid data, overflow)
 *
 * @tparam Integral integer type (signed or unsigned)
 * @param str string to parse
 */
template<typename Integral, std::enable_if_t<std::is_integral<Integral>::value>* = nullptr>
caney::optional<Integral> parse_integral(memory::const_buf const& str) {
	memory::raw_const_buf buf = str.raw_copy();
	caney::optional<Integral> result = parse_integral_open<Integral>(buf);
	return buf.empty() ? result : caney::nullopt;
}

__CANEY_UTILV1_END
