#include "caney/std/bitset.hpp"

__CANEY_STDV1_BEGIN

namespace impl {
	template class bit_mask<uint32_t, bitset_endianness::little_endian>;
	template class bit_mask<uint64_t, bitset_endianness::little_endian>;
	template class bit_mask<uint32_t, bitset_endianness::big_endian>;
	template class bit_mask<uint64_t, bitset_endianness::big_endian>;

	template class bit_reference_base<uint32_t, bit_mask<uint32_t, bitset_endianness::little_endian>>;
	template class bit_reference_base<uint64_t, bit_mask<uint64_t, bitset_endianness::little_endian>>;
	template class bit_reference_base<uint32_t, bit_mask<uint32_t, bitset_endianness::big_endian>>;
	template class bit_reference_base<uint64_t, bit_mask<uint64_t, bitset_endianness::big_endian>>;
	template class bit_reference_base<uint32_t const, bit_mask<uint32_t, bitset_endianness::little_endian>>;
	template class bit_reference_base<uint64_t const, bit_mask<uint64_t, bitset_endianness::little_endian>>;
	template class bit_reference_base<uint32_t const, bit_mask<uint32_t, bitset_endianness::big_endian>>;
	template class bit_reference_base<uint64_t const, bit_mask<uint64_t, bitset_endianness::big_endian>>;
}

__CANEY_STDV1_END
