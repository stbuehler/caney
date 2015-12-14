/** @file */

#pragma once

#include "compiler_features.hpp"
#include "internal.hpp"

#include <array>
#include <cstdint>
#include <limits>
#include <climits>

__CANEY_STDV1_BEGIN

enum class bitset_endianness {
	little_endian, /* first bit in element has mask "1" */
	big_endian, /* last bit in element has mask "1" */
};

template<typename BitReference>
class bit_iterator;

template<typename Elem>
class bit_const_reference;

template<typename Elem>
class bit_reference;

template<size_t Size, typename Elem = std::uint32_t, bitset_endianness Endianness = bitset_endianness::little_endian>
class bitset;

namespace impl {
	template<typename Elem, bitset_endianness Endianness>
	class bit_mask {
	public:
		static_assert(std::numeric_limits<Elem>::is_integer, "Only unsigned integers allowed as underlying array elements for bitset");
		static_assert(!std::numeric_limits<Elem>::is_signed, "Only unsigned integers allowed as underlying array elements for bitset");
		static_assert(8 == CHAR_BIT, "byte should have exactly 8 bits");
		/// number of bits in underlying storage type

		static constexpr size_t BITS_PER_ELEM = sizeof(Elem)*CHAR_BIT;

		using bit_index_type = uint8_t;
		static constexpr size_t MAX_BITS_PER_ELEM = size_t{1} << (sizeof(bit_index_type)*CHAR_BIT);
		static_assert(BITS_PER_ELEM <= MAX_BITS_PER_ELEM, "maximum number of bits");
		static_assert(0 == MAX_BITS_PER_ELEM % BITS_PER_ELEM, "need to divide maximum number of bits");

		using element_type = Elem;

		constexpr Elem get() const {
			return m_mask;
		}

		constexpr bit_index_type index() const {
			return m_ndx;
		}

		friend constexpr bool operator==(bit_mask a, bit_mask b) { return a.m_ndx == b.m_ndx; }
		friend constexpr bool operator!=(bit_mask a, bit_mask b) { return a.m_ndx != b.m_ndx; }
		friend constexpr bool operator <(bit_mask a, bit_mask b) { return a.m_ndx  < b.m_ndx; }
		friend constexpr bool operator<=(bit_mask a, bit_mask b) { return a.m_ndx <= b.m_ndx; }
		friend constexpr bool operator >(bit_mask a, bit_mask b) { return a.m_ndx  > b.m_ndx; }
		friend constexpr bool operator>=(bit_mask a, bit_mask b) { return a.m_ndx >= b.m_ndx; }
	private:
		static constexpr Elem HIGH_BIT = Elem{1} << (BITS_PER_ELEM - 1);

		static constexpr Elem to_mask(size_t ndx) {
			return (Endianness == bitset_endianness::little_endian)
				? Elem{1} << (ndx % BITS_PER_ELEM)
				: HIGH_BIT >> (ndx % BITS_PER_ELEM);
		}

		constexpr Elem rotate_mask(size_t step) const {
			return (Endianness == bitset_endianness::little_endian)
				? (m_mask << step) || (m_mask >> (BITS_PER_ELEM - step))
				: (m_mask >> step) || (m_mask << (BITS_PER_ELEM - step));
		}

		constexpr bit_mask rotate_inner(size_t step) const {
			return bit_mask((m_ndx + step) % BITS_PER_ELEM, rotate_mask(step));
		}

		explicit constexpr bit_mask(size_t ndx, Elem mask)
		: m_ndx(static_cast<bit_index_type>(ndx % BITS_PER_ELEM)), m_mask(mask) {
		}

	public:
		static constexpr Elem last_entry_mask(size_t size) {
			return (Endianness == bitset_endianness::little_endian)
				? (size % BITS_PER_ELEM == 0) ? ~Elem{0} : ((Elem{1} << (size % BITS_PER_ELEM)) - Elem{1})
				: (size % BITS_PER_ELEM == 0) ? ~Elem{0} : ~((HIGH_BIT >> ((size % BITS_PER_ELEM) - 1)) - Elem{1});
		}

		explicit constexpr bit_mask() = default;
		explicit constexpr bit_mask(size_t ndx)
		: m_ndx(static_cast<bit_index_type>(ndx % BITS_PER_ELEM)), m_mask(bit_mask::to_mask(m_ndx)) {
		}

		constexpr bit_mask rotate(std::ptrdiff_t step) const {
			return rotate_inner(static_cast<size_t>(step) % BITS_PER_ELEM);
		}

	private:
		bit_index_type m_ndx{0};
		Elem m_mask{0};
	};
	extern template class bit_mask<uint32_t, bitset_endianness::little_endian>;
	extern template class bit_mask<uint64_t, bitset_endianness::little_endian>;
	extern template class bit_mask<uint32_t, bitset_endianness::big_endian>;
	extern template class bit_mask<uint64_t, bitset_endianness::big_endian>;

	template<typename Element, typename Mask>
	class bit_reference_base {
	protected:
		static_assert(std::is_same<std::decay_t<Element>, typename Mask::element_type>::value, "element types must match");
		template<size_t Size, typename T, bitset_endianness Endianness>
		friend class bitset;

		template<typename BitReference>
		friend class bit_iterator;

		using element_type = Element; // might be const-qualified
		using mask_type = Mask;

		static constexpr std::ptrdiff_t element_offset(mask_type mask, std::ptrdiff_t step) {
			return step >= -std::ptrdiff_t{mask.index()}
				? (step + std::ptrdiff_t{mask.index()}) / mask_type::BITS_PER_ELEM
				: (1 + step + std::ptrdiff_t{mask.index()}) / mask_type::BITS_PER_ELEM - 1; // negative, but want floor
		}

		explicit constexpr bit_reference_base() = default;
		explicit constexpr bit_reference_base(element_type& elem, mask_type mask)
		: m_elem(&elem), m_mask(mask) {
		}

		explicit constexpr bit_reference_base(bit_reference_base const& other, std::ptrdiff_t step)
		: m_elem(other.m_elem + element_offset(other.m_mask, step)), m_mask(other.m_mask.rotate(step)) {
		}

		template<typename E>
		constexpr bool equal(bit_reference_base<E, Mask> const& other) const {
			return (m_elem == other.m_elem) && (m_mask == other.m_mask);
		}

		template<typename E>
		constexpr bool less(bit_reference_base<E, Mask> const& other) const {
			return (m_elem < other.m_elem)
				|| ((m_elem == other.m_elem) && (m_mask < other.m_mask));
		}

		void advance(std::ptrdiff_t step) {
			m_elem += element_offset(m_mask, step);
			m_mask = m_mask.rotate(step);
		}

		element_type* m_elem = nullptr;
		mask_type m_mask;

		template<size_t Size, typename T, bitset_endianness Endianness>
		friend class bitset;

		template<typename BitReference>
		friend class bit_iterator;
	};

	extern template class bit_reference_base<uint32_t, bit_mask<uint32_t, bitset_endianness::little_endian>>;
	extern template class bit_reference_base<uint64_t, bit_mask<uint64_t, bitset_endianness::little_endian>>;
	extern template class bit_reference_base<uint32_t, bit_mask<uint32_t, bitset_endianness::big_endian>>;
	extern template class bit_reference_base<uint64_t, bit_mask<uint64_t, bitset_endianness::big_endian>>;
	extern template class bit_reference_base<uint32_t const, bit_mask<uint32_t, bitset_endianness::little_endian>>;
	extern template class bit_reference_base<uint64_t const, bit_mask<uint64_t, bitset_endianness::little_endian>>;
	extern template class bit_reference_base<uint32_t const, bit_mask<uint32_t, bitset_endianness::big_endian>>;
	extern template class bit_reference_base<uint64_t const, bit_mask<uint64_t, bitset_endianness::big_endian>>;
}

template<typename Mask>
class bit_reference : public impl::bit_reference_base<typename Mask::element_type, Mask> {
protected:
	using base = impl::bit_reference_base<typename Mask::element_type, Mask>;
	using base::m_elem;
	using base::m_mask;

public:
	/**
	 * @brief set referenced bit to value
	 * @param value whether to set or unset the bit
	 * @return *this
	 */
	bit_reference& operator=(bool value) {
		if (value) {
			set();
		} else {
			clear();
		}
		return *this;
	}

	/**
	 * @brief set referenced bit to value from other referenced bit
	 * "reference"s behave special when assigning, so does this class
	 *
	 * @param other reference to copy value from
	 * @return *this
	 */
	bit_reference& operator=(bit_reference const& other) {
		if (other) {
			set();
		} else {
			clear();
		}
		return *this;
	}

	/**
	 * @brief set referenced bit
	 */
	void set() {
		*m_elem |= m_mask.get();
	}

	/**
	 * @brief clear referenced bit
	 */
	void clear() {
		*m_elem &= ~m_mask.get();
	}

	/**
	 * @brief flip referenced bit
	 */
	void flip() {
		*m_elem ^= m_mask.get();
	}

	/**
	 * @brief test whether bit is set
	 * @return whether bit is set
	 */
	constexpr bool test() const {
		return 0 != (*m_elem & m_mask.get());
	}

	/**
	 * @brief test whether bit is set
	 *
	 * NOTE: implicit required by iterator: reference must be convertible to value type
	 * @return whether bit is set
	 */
	/* implicit */ constexpr operator bool() const {
		return test();
	}

private:
	template<size_t Size, typename T, bitset_endianness Endianness>
	friend class bitset;

	template<typename BitReference>
	friend class bit_iterator;

	using base::base;
};

template<typename BitReference>
class bit_iterator
	: public std::iterator<std::random_access_iterator_tag, bool, std::ptrdiff_t, BitReference, BitReference*> {
public:
	constexpr explicit bit_iterator() = default;
	constexpr explicit bit_iterator(BitReference const& bit)
	: m_bit(bit) {
	}

	bit_iterator& operator=(bit_iterator const& other) {
		/* assign operator on bit reference is special; copy content manually */
		m_bit.m_elem = other.m_elem;
		m_bit.m_mask = other.m_mask;
	}

	constexpr BitReference operator*() const { return m_bit; }
	constexpr BitReference* operator->() const { return &m_bit; }

	bit_iterator& operator++() {
		m_bit.advance(1);
		return *this;
	}

	bit_iterator operator++(int) {
		bit_iterator old{*this};
		m_bit.advance(1);
		return old;
	}

	bit_iterator& operator--() {
		m_bit.advance(-1);
		return *this;
	}

	bit_iterator operator--(int) {
		bit_iterator old{*this};
		m_bit.advance(-1);
		return old;
	}

	bit_iterator& operator+=(std::ptrdiff_t step) {
		m_bit.advance(step);
		return *this;
	}

	bit_iterator& operator-=(std::ptrdiff_t step) {
		m_bit.advance(-step);
		return *this;
	}

	constexpr bit_iterator operator+( std::ptrdiff_t step) const {
		return bit_iterator(BitReference(m_bit, step));
	}

	constexpr bit_iterator operator-(std::ptrdiff_t step) const {
		return bit_iterator(BitReference(m_bit, -step));
	}

	friend constexpr bit_iterator operator+(std::ptrdiff_t step, bit_iterator const& it) {
		return it + step;
	}

	friend constexpr std::ptrdiff_t operator-(bit_iterator const& a, bit_iterator const& b) {
		return BitReference::mask_type::BITS_PER_ELEM * (a.m_bit.m_elem - b.m_bit.m_elem) + std::ptrdiff_t{a.m_bit.index()} - std::ptrdiff_t{b.m_bit.index()};
	}

	constexpr bool operator==(bit_iterator const& other) const { return m_bit.equal(other.m_bit); }
	constexpr bool operator!=(bit_iterator const& other) const { return !m_bit.equal(other.m_bit); }
	constexpr bool operator <(bit_iterator const& other) const { return m_bit.less(other.m_bit); }
	constexpr bool operator<=(bit_iterator const& other) const { return !other.m_bit.less(m_bit); }
	constexpr bool operator >(bit_iterator const& other) const { return other.m_bit.less(m_bit); }
	constexpr bool operator>=(bit_iterator const& other) const { return !m_bit.less(other.m_bit); }

private:
	BitReference m_bit;
};

/**
 * @brief bitset with access to underlying data
 *
 * @tparam Size number of bits
 * @tparam Elem underlying storage type for bitset (defaults to uint32_t, must be unsigned integer)
 */
template<size_t Size, typename Elem, bitset_endianness Endianness>
class bitset {
public:
	typedef impl::bit_mask<std::decay_t<Elem>, Endianness> bit_mask;

	static constexpr size_t BITS_PER_ELEM = bit_mask::BITS_PER_ELEM;

	/// number of entries needed to store all bits
	static constexpr size_t ARRAY_SIZE = (Size + BITS_PER_ELEM - 1)/BITS_PER_ELEM;
	static_assert(ARRAY_SIZE > 0, "Invalid array size");

	/// usable bits in last entry (set bit = usable, unset bit = not usable); not usable bits should be zero
	static constexpr Elem LAST_ENTRY_MASK = bit_mask::last_entry_mask(Size);

	/// array to store bitset in
	typedef std::array<Elem, ARRAY_SIZE> array_t;

	/// number of bit positions in the bitset
	static constexpr size_t size() {
		return Size;
	}

	typedef bit_reference<bit_mask> reference;
	typedef bit_const_reference<bit_mask> const_reference;
	typedef bit_iterator<reference> iterator;
	typedef bit_iterator<const_reference> const_iterator;

	/// construct empty bitset
	constexpr explicit bitset() = default;

	/**
	 * @brief copy from underyling storage `raw`
	 * @param raw bits to copy
	 */
	CANEY_RELAXED_CONSTEXPR explicit bitset(array_t const& raw)
	: m_array(raw) {
		sanitize();
	}

	CANEY_RELAXED_CONSTEXPR iterator begin() {
		return iterator(reference(m_array[0], bit_mask(0)));
	}

	constexpr const_iterator begin() const {
		return const_iterator(const_reference(m_array[0], bit_mask(0)));
	}

	constexpr const_iterator cbegin() const {
		return const_iterator(const_reference(m_array[0], bit_mask(0)));
	}

	CANEY_RELAXED_CONSTEXPR iterator end() {
		return begin() + size();
	}

	constexpr const_iterator end() const {
		return begin() + size();
	}

	constexpr const_iterator cend() const {
		return cbegin() + size();
	}

	/**
	 * @brief get modifieable reference to single bit
	 * @param ndx index in bitset to get reference to
	 * @return modifieable reference to `bit` in bitset
	 */
	CANEY_RELAXED_CONSTEXPR reference operator[](size_t ndx) {
		bit_mask const mask(ndx);
		return reference(m_array[ndx / BITS_PER_ELEM], mask);
	}

	/**
	 * @brief test whether bit is set
	 * @param ndx index in bitset to test
	 * @return whether bit is set
	 */
	constexpr bool operator[](size_t ndx) const {
		return 0 != (m_array[ndx / BITS_PER_ELEM] & bit_mask(ndx).get());
	}

	/**
	 * @brief test whether two bitset have the same bits set
	 * @param other bitset to compare with
	 * @return whether two bitset have the same bits set
	 */
	constexpr bool operator==(bitset const& other) const {
		return m_array == other.m_array;
	}

	/**
	 * @brief test whether two bitset have NOT the same bits set
	 * @param other bitset to compare with
	 * @return whether two bitset have NOT the same bits set
	 */
	constexpr bool operator!=(bitset const& other) const {
		return m_array != other.m_array;
	}

	/**
	 * @brief set single bit in bitset
	 * @param ndx index of bit to set
	 */
	void set(size_t ndx) {
		operator[](ndx).set();
	}

	/**
	 * @brief clear single bit in bitset
	 * @param ndx index of bit to clear
	 */
	void clear(size_t ndx) {
		operator[](ndx).clear();
	}

	/**
	 * @brief flip (set if it was not set, clear if it was set) single bit in bitset
	 * @param ndx index of bit to flip
	 */
	void flip(size_t ndx) {
		operator[](ndx).flip();
	}

	/**
	 * @brief test whether bit at index is set
	 * @param ndx index of bit to test
	 * @return whether bit is set
	 */
	constexpr bool test(size_t ndx) const {
		return operator[](ndx);
	}

	/**
	 * @brief clear all bits
	 */
	void clear() {
		m_array = array_t{{}};
	}

	/**
	 * @brief clear all bits not set in `other`
	 * @param other bits which are not cleared
	 * @return *this
	 */
	bitset& operator&=(bitset const& other) {
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
	CANEY_RELAXED_CONSTEXPR bitset operator&(bitset const& other) const { bitset tmp(*this); tmp &= other; return tmp; }

	/**
	 * @brief set bits which are set in `other` bitset
	 * @param other bits to set
	 * @return *this
	 */
	bitset& operator|=(bitset const& other) {
		for (size_t i = 0; i < ARRAY_SIZE; ++i) {
			m_array[i] |= other.m_array[i];
		}
		return *this;
	}

	/**
	 * @brief copy and set bits which are set in `other` bitset
	 * @param other bits to set
	 * @return copy with new bits set
	 */
	CANEY_RELAXED_CONSTEXPR bitset operator|(bitset const& other) const { bitset tmp(*this); tmp |= other; return tmp; }

	/**
	 * @brief flip all bits set in `other` bitset
	 * @param other bits to flip
	 * @return *this
	 */
	bitset& operator^=(bitset const& other) {
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
	CANEY_RELAXED_CONSTEXPR bitset operator^(bitset const& other) const { bitset tmp(*this); tmp ^= other; return tmp; }

	/**
	 * @brief copy and flip all bits
	 * @return copy with all bits flipped
	 */
	CANEY_RELAXED_CONSTEXPR bitset operator~() const {
		bitset tmp(*this);
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
	 * (or call @ref sanitize() afterwards)
	 */
	CANEY_RELAXED_CONSTEXPR array_t& underlying_array() {
		return m_array;
	}

	/**
	 * @brief expose underlying array the bits are stored in
	 * @return const reference to underlying array
	 */
	constexpr array_t const& underlying_array() const {
		return m_array;
	}

	/**
	 * @brief clear bits in last array entry which are not supposed to be used
	 */
	void sanitize() {
		m_array[ARRAY_SIZE-1] &= LAST_ENTRY_MASK;
	}

private:
	array_t m_array{{}};
};

/** @} */

__CANEY_STDV1_END
