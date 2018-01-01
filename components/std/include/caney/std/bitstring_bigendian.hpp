/** @file */

#pragma once

#include "internal.hpp"

#include <algorithm>

#include <cassert>

#include <stddef.h>
#include <stdint.h>

__CANEY_STDV1_BEGIN

/** @namespace bigendian */
namespace bigendian {
	/**
	 * provides a @ref bitstring implementation on a bigendian interpration
	 * of binary data; does not keep the (referenced) data alive.
	 *
	 * big endian means: the first bit in the bitstring it the most significant bit (0x80) in the first byte.
	 *
	 * This translates for example to the the first bit in the network (big endian) representation of an IP address.
	 */
	class bitstring {
	public:
		/**
		 * @brief construct empty bitstring
		 */
		explicit constexpr bitstring() noexcept = default;

		/**
		 * @brief construct bitstring from raw memory (interpreted as sequence of bytes);
		 * memory needs to be at least (length+7)/8 bytes long, length is the
		 * length of the bitstring in bits
		 */
		explicit constexpr bitstring(void const* data, size_t length) noexcept
		: m_data(data), m_length(length) {
		}

		/**
		 * @brief @ref bitstring required `length` method
		 * @returns length of bitstring in bits
		 */
		size_t length() const { return m_length; }

		/**
		 * @brief @ref bitstring required `truncate` method
		 * @returns truncates length of bitstring to at most `length` bits (doesn't append new bits)
		 */
		bitstring truncate(size_t length) const {
			return bitstring(m_data, std::min(length, m_length));
		}

		/**
		 * @brief @ref bitstring required index operator
		 * @returns whether bit at requested index is set
		 */
		bool operator[](size_t bit_ndx) const;

		/**
		 * @brief returns mask of bits possibly set in the last byte (@ref fraction_byte) for a bitstring of given length
		 */
		static unsigned char content_mask(size_t length);

		/**
		 * @brief store bitstring data in given memory, which must provide space for at least `(length()+7)/8` bytes.
		 * Remaining bits and bytes will be filled with zeroes.
		 *
		 * `data` must not overlap with the underlying bitstring storage, with the exception of pointing
		 * to the first byte of it. in other words: `bitstring(data, length).set_bitstring(data, size);` is ok.
		 */
		void set_bitstring(void* data, size_t data_size) const;

		/** get internal data pointer as byte pointer */
		unsigned char const* byte_data() const {
			return reinterpret_cast<unsigned char const*>(m_data);
		}

		/** `byte_data()[byte_ndx]` */
		unsigned char get_byte(size_t byte_ndx) const;

		/** get bit with original value in byte, i.e. returns 0 or (0x100u >> (ndx % 8)) */
		unsigned char get_bit(size_t bit_ndx) const;

		/**
		 * return bits of last (incomplete) byte (masks out unused bits);
		 * returns 0 if there is no incomplete byte (`0 == length() % 8`)
		 */
		unsigned char fraction_byte() const;

	private:
		void const* m_data{nullptr};
		size_t m_length{0};
	};

	/** @ref bitstring required equal check */
	bool operator==(bitstring const& a, bitstring const& b);
	/** @ref bitstring required not-equal check */
	bool operator!=(bitstring const& a, bitstring const& b);

	/** @ref bitstring required lexicographic ordering */
	bool is_lexicographic_less(bitstring const& a, bitstring const& b);

	/** @ref bitstring required tree ordering */
	bool is_tree_less(bitstring const& a, bitstring const& b);

	/** @ref bitstring required prefix test */
	bool is_prefix(bitstring const& prefix, bitstring const& str);

	/** @ref bitstring required calculation of longest common prefix. uses internal data pointer from `a` in result */
	bitstring longest_common_prefix(bitstring const& a, bitstring const& b);
}

__CANEY_STDV1_END
