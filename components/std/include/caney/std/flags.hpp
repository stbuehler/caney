/** @file */

#pragma once

#include "compiler_features.hpp"
#include "internal.hpp"

#include <array>
#include <cstdint>
#include <limits>
#include <climits>

__CANEY_STDV1_BEGIN

/**
 * @defgroup flags flags
 * Flags construct a bitset from a list of flags (given as enum members).
 *
 * The base enum defines the indices to look up in a bitset; you should use an enum class so there are no predefined (and wrong) bit-wise operators.
 *
 * Bit operations |, & and ^ usually work on the complete bitset; only &, when one parameter is a single flag, will simply test whether the flag is set and return a boolean.
 *
 * Example:
 * @code
 *     enum class my_flag : size_t {
 *         foo0 = 0,
 *         foo1 = 1,
 *         // ...
 *         foo10 = 10,
 *         Last = foo10,
 *     };
 *
 *     using my_flags = caney::flags<my_flag>;
 *     CANEY_FLAGS(my_flags)
 * @endcode
 *
 * @addtogroup flags
 * @{
 */

/**
 * @brief Define standard bitwise operators on `Flags`.
 */
#define CANEY_FLAGS(Flags) \
	/**
	 * @brief return bitset where flag `a` and `b` are set
	 * @param a single flag
	 * @param b single flag
	 * @return bitset
	 */ \
	Flags operator|(Flags::flag_t a, Flags::flag_t b) { return Flags(a) | b; } \
	/**
	 * @brief return bitset where flag `a` and all set bits in `b` are set
	 * @param a single flag
	 * @param b bitset
	 * @return bitset
	 */ \
	Flags operator|(Flags::flag_t a, Flags b) { return Flags(a) | b; } \
	/**
	 * @brief deleted
	 * @internal
	 */ \
	bool operator&(Flags::flag_t a, Flags::flag_t b) = delete; \
	/**
	 * @brief check whether `a` is set in b
	 * @param a single flag
	 * @param b bitset
	 * @return whether `a` is set in b
	 */ \
	bool operator&(Flags::flag_t a, Flags b) { return b & a; } \
	/**
	 * @brief toggle `a` in bitset from single flag `b` (or vice versa)
	 * @param a single flag
	 * @param b single flag
	 * @return empty bitset if `a == b` otherwise `a | b`
	 */ \
	Flags operator^(Flags::flag_t a, Flags::flag_t b) { return Flags(a) ^ b; } \
	/**
	 * @brief toggle `a` in `b`
	 * @param a single flag
	 * @param b bitset
	 * @return `b` but with flag `a` negated
	 */ \
	Flags operator^(Flags::flag_t a, Flags b) { return Flags(a) ^ b; } \
	/**
	 * @brief negate all bits
	 * @param a bitset
	 * @return inverse to `a`
	 */ \
	Flags operator~(Flags::flag_t a) { return ~Flags(a); }

namespace impl {
	// idea from http://stackoverflow.com/a/10724828/1478356
	template<typename Enum, bool B = std::is_enum<Enum>::value>
	struct is_scoped_enum : std::false_type {
	};

	template<typename Enum>
	struct is_scoped_enum<Enum, true>
		: std::integral_constant<
			bool,
			!std::is_convertible<Enum, std::underlying_type_t<Enum>>::value> {
	};
}

/**
 * @brief bitset representing possible set flags from enum
 *
 * @tparam FlagEnum scoped enum with consecutive flag values
 * @tparam Size index of smallest invalid flag value (the first after all flag values), only required if `FlagEnum` doesn't contain a correct `Last` member
 * @tparam Elem underlying storage type for bitset (defaults to uint32_t, must be unsigned integer)
 */
template<typename FlagEnum, std::size_t Size = static_cast<std::size_t>(FlagEnum::Last) + 1, typename Elem = std::uint32_t>
class flags {
private:
	typedef std::underlying_type_t<FlagEnum> enum_t;
public:
	static_assert(impl::is_scoped_enum<FlagEnum>::value, "Only scoped enums allowed");
	static_assert(!std::numeric_limits<enum_t>::is_signed, "Only enums with unsigned underlying types allowed");
	static_assert(std::numeric_limits<Elem>::is_integer, "Only unsigned integers allowed as underlying array elements for flags");
	static_assert(!std::numeric_limits<Elem>::is_signed, "Only unsigned integers allowed as underlying array elements for flags");

	static_assert(8 == CHAR_BIT, "byte should have exactly 8 bits");
	/// number of bits in underlying storage type
	static constexpr std::size_t BITS_PER_ELEM = sizeof(Elem)*CHAR_BIT;

	/// number of entries needed to store all bits
	static constexpr std::size_t ARRAY_SIZE = (Size + BITS_PER_ELEM - 1)/BITS_PER_ELEM;
	static_assert(ARRAY_SIZE > 0, "Invalid array size");

	/// usable bits in last entry (set bit = usable, unset bit = not usable); not usable bits should be zero
	static constexpr Elem LAST_ENTRY_MASK = (Size % BITS_PER_ELEM == 0) ? ~Elem{0} : ((Elem{1} << (Size % BITS_PER_ELEM)) - Elem{1});

	/// array to store bitset in
	typedef std::array<Elem, ARRAY_SIZE> array_t;

	/// typedef for single flag (scoped enum)
	typedef FlagEnum flag_t;

	/// number of bit positions in the bitset
	static constexpr std::size_t size() {
		return Size;
	}

	/** reference to a single flag (bit) in the bitset */
	class reference {
	public:
		/**
		 * @brief set referenced flag to value
		 * @param value whether to set or unset the flag
		 * @return *this
		 */
		reference& operator=(bool value) {
			if (value) {
				set();
			} else {
				clear();
			}
			return *this;
		}

		/**
		 * @brief set referenced flag (bit in underlying bitset)
		 */
		void set() {
			*m_elem |= m_mask;
		}

		/**
		 * @brief clear referenced flag (bit in underlying bitset)
		 */
		void clear() {
			*m_elem &= ~m_mask;
		}

		/**
		 * @brief flip referenced flag (bit in underlying bitset)
		 */
		void flip() {
			*m_elem ^= m_mask;
		}

		/**
		 * @brief test whether flag is set
		 * @return whether flag is set
		 */
		bool test() const {
			return 0 != (*m_elem & m_mask);
		}

		/**
		 * @brief test whether flag is set
		 * @return whether flag is set
		 */
		explicit operator bool() const {
			return test();
		}

	private:
		friend class flags<FlagEnum, Size, Elem>;
		explicit reference(Elem& elem, Elem mask)
		: m_elem(&elem), m_mask(mask) {
		}

		Elem* m_elem;
		Elem m_mask;
	};

	/// construct empty bitset
	explicit flags() = default;
	/**
	 * @brief copy from underyling storage `raw`
	 * @param raw bits to copy
	 */
	explicit flags(array_t const& raw)
	: m_array(raw) {
	}

	/**
	 * @brief construct bitset containing a single flag (set a single bit)
	 * @param flag bit to set in bitset
	 */
	flags(flag_t flag)
	{
		set(flag);
	}

	/**
	 * @brief get modifieable reference to single flag
	 * @param flag index in bitset to get reference to
	 * @return modifieable reference to `flag` in bitset
	 */
	reference operator[](flag_t flag) {
		size_t flagNdx{static_cast<enum_t>(flag)};
		Elem const mask = Elem{1} << (flagNdx % BITS_PER_ELEM);
		return reference(m_array[flagNdx / BITS_PER_ELEM], mask);
	}

	/**
	 * @brief test whether flag is set
	 * @param flag index in bitset to test
	 * @return whether flag is set
	 */
	CANEY_RELAXED_CONSTEXPR bool operator[](flag_t flag) const {
		size_t flagNdx{static_cast<enum_t>(flag)};
		Elem const mask = Elem{1} << (flagNdx % BITS_PER_ELEM);
		return 0 != (m_array[flagNdx / BITS_PER_ELEM] & mask);
	}

	/**
	 * @brief test whether two bitset have the same bits set
	 * @param other bitset to compare with
	 * @return whether two bitset have the same bits set
	 */
	constexpr bool operator==(flags const& other) const {
		return m_array == other.m_array;
	}

	/**
	 * @brief test whether two bitset have NOT the same bits set
	 * @param other bitset to compare with
	 * @return whether two bitset have NOT the same bits set
	 */
	constexpr bool operator!=(flags const& other) const {
		return m_array != other.m_array;
	}

	/**
	 * @brief set single flag in bitset
	 * @param flag to set
	 */
	void set(flag_t flag) {
		operator[](flag).set();
	}

	/**
	 * @brief clear single flag in bitset
	 * @param flag to clear
	 */
	void clear(flag_t flag) {
		operator[](flag).clear();
	}

	/**
	 * @brief flip (set if it was not set, clear if it was set) single flag in bitset
	 * @param flag to flip
	 */
	void flip(flag_t flag) {
		operator[](flag).flip();
	}

	/**
	 * @brief test whether flag is set
	 * @param flag index in bitset to test
	 * @return whether flag is set
	 */
	CANEY_RELAXED_CONSTEXPR bool test(flag_t flag) const {
		return operator[](flag);
	}

	/**
	 * @brief clear all bits
	 */
	void clear() {
		m_array = array_t{{}};
	}

	/**
	 * @brief deleted
	 * @internal
	 */
	bool operator&=(flag_t) = delete;

	/**
	 * @brief test whether flag is set
	 * @param flag index in bitset to test
	 * @return whether flag is set
	 */
	CANEY_RELAXED_CONSTEXPR bool operator&(flag_t flag) const {
		return test(flag);
	}

	/**
	 * @brief clear all bits not set in `other`
	 * @param other bits which are not cleared
	 * @return *this
	 */
	flags& operator&=(flags const& other) {
		for (size_t i = 0; i < ARRAY_SIZE; ++i) {
			m_array[i] &= other.m_array[i];
		}
		return *this;
	}

	/**
	 * @brief copy and clear all bits not set in `other`
	 * @param other bits which are not cleared
	 * @return copy with bits not in `other` cleared
	 */
	CANEY_RELAXED_CONSTEXPR flags operator&(flags const& other) const { flags tmp(*this); tmp &= other; return tmp; }

	/**
	 * @brief set single flag in bitset
	 * @param flag to set
	 */
	flags& operator|=(flag_t flag) {
		set(flag);
		return *this;
	}

	/**
	 * @brief set flags which are set in `other` bitset
	 * @param other bits to set
	 * @return *this
	 */
	flags& operator|=(flags const& other) {
		for (size_t i = 0; i < ARRAY_SIZE; ++i) {
			m_array[i] |= other.m_array[i];
		}
		return *this;
	}

	/**
	 * @brief copy and set flags which are set in `other` bitset
	 * @param other bits to set
	 * @return copy with new bits set
	 */
	CANEY_RELAXED_CONSTEXPR flags operator|(flags const& other) const { flags tmp(*this); tmp |= other; return tmp; }

	/**
	 * @brief flip single flag in bitset
	 * @param flag to flip
	 */
	flags& operator^=(flag_t flag) {
		flip(flag);
		return *this;
	}

	/**
	 * @brief flip all bits set in `other` bitset
	 * @param other bits to flip
	 * @return *this
	 */
	flags& operator^=(flags const& other) {
		for (size_t i = 0; i < ARRAY_SIZE; ++i) {
			m_array[i] ^= other.m_array[i];
		}
		return *this;
	}

	/**
	 * @brief copy and flip all bits set in `other` bitset
	 * @param other bits to flip
	 * @return copy with flipped bits
	 */
	CANEY_RELAXED_CONSTEXPR flags operator^(flags const& other) const { flags tmp(*this); tmp ^= other; return tmp; }

	/**
	 * @brief copy and flip all bits
	 * @return copy with all bits flipped
	 */
	CANEY_RELAXED_CONSTEXPR flags operator~() const {
		flags tmp(*this);
		tmp.flip_all();
		return tmp;
	}

	/**
	 * @brief flip all bits
	 */
	void flip_all() {
		for (size_t i = 0; i < ARRAY_SIZE - 1; ++i) {
			m_array[i] = ~m_array[i];
		}
		m_array[ARRAY_SIZE-1] ^= LAST_ENTRY_MASK;
	}

	/**
	 * @brief test whether no bit is set
	 * @return whether no bit is set
	 */
	constexpr bool none() const {
		return m_array == array_t{{}};
	}

	/**
	 * @brief test whether at least one bit is set
	 * @return whether at least one bit is set
	 */
	constexpr bool any() const {
		return !none();
	}

	/**
	 * @brief test whether all bits are set
	 * @return whether all bits are set
	 */
	CANEY_RELAXED_CONSTEXPR bool all() const {
		for (size_t i = 0; i < ARRAY_SIZE - 1; ++i) {
			if (0 != ~m_array[i]) return false;
		}
		return m_array[ARRAY_SIZE-1] == LAST_ENTRY_MASK;
	}

	/**
	 * @brief expose underlying array the bits are stored in
	 * @return reference to underlying array
	 *
	 * NOTE: don't set bits in last array entry which are not allowed according to @ref LAST_ENTRY_MASK !
	 */
	array_t& underlying_array() {
		return m_array;
	}

	/**
	 * @brief expose underlying array the bits are stored in
	 * @return const reference to underlying array
	 */
	array_t const& underlying_array() const {
		return m_array;
	}

private:
	array_t m_array{{}};
};

#if defined(DOXYGEN)
	CANEY_FLAGS(flags<FlagEnum>)
#endif

/** @} */

__CANEY_STDV1_END
