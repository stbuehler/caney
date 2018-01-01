#include <caney/std/bitstring_network.hpp>

__CANEY_STDV1_BEGIN

bool operator==(network_v4_bitstring const& a, network_v4_bitstring const& b) {
	return a.network().length() == b.network().length() && a.network().address() == b.network().address();
}

bool operator!=(network_v4_bitstring const& a, network_v4_bitstring const& b) {
	return !(a == b);
}

bool is_lexicographic_less(network_v4_bitstring const& a, network_v4_bitstring const& b) {
	if (a.network().native_address() == b.network().native_address()) {
		// one is prefix of the other; shorter comes first
		return a.network().length() < b.network().length();
	}
	// only the smaller one can be a "real" prefix of the other
	return a.network().native_address() < b.network().native_address();
}

bool is_tree_less(network_v4_bitstring const& a, network_v4_bitstring const& b) {
	unsigned char trunc_len = std::min(a.network().length(), b.network().length());
	uint32_t const truncate_mask = network_v4::native_netmask(trunc_len);
	uint32_t const a_native_address_trunc = truncate_mask & a.network().native_address();
	uint32_t const b_native_address_trunc = truncate_mask & b.network().native_address();
	if (a_native_address_trunc < b_native_address_trunc) return true;
	if (a_native_address_trunc == b_native_address_trunc) {
		// one is prefix of the other
		if (a.network().length() == b.network().length()) return false; // a == b
		// 1 <= trunc_len+1 <= 32
		uint32_t const native_next_bit_mask = uint32_t{1} << (32 - (trunc_len+1));
		if (a.network().length() < b.network().length()) { // a prefix of b
			return (native_next_bit_mask & b.network().native_address()) > 0;
		} else { // b prefix of a
			return (native_next_bit_mask & a.network().native_address()) > 0;
		}
	}
	return false;
}

bool is_prefix(network_v4_bitstring const& prefix, network_v4_bitstring const& str) {
	return prefix == str.truncate(prefix.length());
}

#if !defined(__has_builtin)
/**
 * @brief fake __has_builtin macro if not available
 * @internal
 */
# define __has_builtin(x) 0
#endif

network_v4_bitstring longest_common_prefix(network_v4_bitstring const& a, network_v4_bitstring const& b) {
	uint32_t native_uncommon_bits = (a.network().native_address() ^ b.network().native_address()) | network_v4::native_hostmask(std::min(a.network().length(), b.network().length()));
#if defined(__GNUC__) || (__has_builtin(__builtin_clz) && __has_builtin(__builtin_clzl))
	size_t length;
	if (sizeof(unsigned int) >= sizeof(uint32_t)) {
		length = static_cast<size_t>(__builtin_clz(native_uncommon_bits)) - 8*(sizeof(unsigned int) - sizeof(uint32_t));
	} else {
		static_assert(sizeof(unsigned long) >= sizeof(uint32_t), "sizeof(unsigned long) violates standard requirements");
		length = static_cast<size_t>(__builtin_clzl(native_uncommon_bits)) - 8*(sizeof(unsigned long) - sizeof(uint32_t));
	}
#else
	uint32_t native_common_bits = ~native_uncommon_bits;
	size_t length = 0;
	for (uint32_t native_bit = uint32_t{1} << 31; 0 != (native_bit & native_common_bits); native_bit >>= 1, ++length) ;
#endif
	return a.truncate(length);
}

__CANEY_STDV1_END
