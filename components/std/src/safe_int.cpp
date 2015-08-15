#include "caney/std/safe_int.hpp"

namespace caney {
	inline namespace stdv1 {
		template class safe_int<char>;
		template class safe_int<int8_t>;
		template class safe_int<uint8_t>;
		template class safe_int<int16_t>;
		template class safe_int<uint16_t>;
		template class safe_int<int32_t>;
		template class safe_int<uint32_t>;
		template class safe_int<int64_t>;
		template class safe_int<uint64_t>;
	}
}
