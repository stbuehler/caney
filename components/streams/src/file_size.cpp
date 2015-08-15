#include "caney/streams/file_size.hpp"

namespace caney {
	template class safe_int<uint64_t, streams::impl::file_size_tag>;
}
