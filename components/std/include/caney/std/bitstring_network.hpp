/** @file */

#pragma once

#include "internal.hpp"
#include "network.hpp"

__CANEY_STDV1_BEGIN

/** @ref bitstring implementation for IPv4 network */
class network_v4_bitstring {
public:
	/** use default constructed @ref network_v4 value */
	network_v4_bitstring() = default;

	/** provide bitstring for given @ref network_v4 value */
	explicit network_v4_bitstring(network_v4 network)
	: m_value(network) {
	}

	/** network length of internal value */
	size_t length() const { return m_value.length(); }

	/** shorten bitstring to at most `length` (i.e. covers a larger IPv4 network) */
	network_v4_bitstring truncate(size_t length) const {
		unsigned char const new_length = static_cast<unsigned char>(std::min<size_t>(length, m_value.length()));
		return network_v4_bitstring(network_v4(m_value.address(), new_length));
	}

	/** read bit at given index `ndx` */
	bool operator[](size_t ndx) const {
		uint32_t const native_mask = uint32_t{1} << (31u - (ndx % 32u));
		return 0 != (m_value.native_address() & native_mask);
	}

	/** access internal @ref network_v4 value */
	const network_v4& network() const {
		return m_value;
	}

private:
	network_v4 m_value{};
};

/**
 * @{
 * @ref bitstring concept operations on @ref network_v4_bitstring
 */
bool operator==(network_v4_bitstring const& a, network_v4_bitstring const& b);
bool operator!=(network_v4_bitstring const& a, network_v4_bitstring const& b);

bool is_lexicographic_less(network_v4_bitstring const& a, network_v4_bitstring const& b);

bool is_tree_less(network_v4_bitstring const& a, network_v4_bitstring const& b);

bool is_prefix(network_v4_bitstring const& prefix, network_v4_bitstring const& str);

network_v4_bitstring longest_common_prefix(network_v4_bitstring const& a, network_v4_bitstring const& b);
/** @} */

/** helper traits for using @ref network_v4 as key in @ref radix_tree */
struct network_v4_bitstring_traits {
	/** bitstring type */
	typedef network_v4_bitstring bitstring;
	/** value type (that is: key in @ref radix_tree) */
	typedef network_v4 value_type;

	/** convert value to bitstring */
	bitstring value_to_bitstring(value_type val) {
		return bitstring(val);
	}

	/** convert bitstring to value */
	value_type bitstring_to_value(bitstring bs) {
		return bs.network();
	}
};

__CANEY_STDV1_END
