#pragma once

#include "caney/std/safe_int.hpp"

namespace caney {
	namespace streams {
		inline namespace v1 {
			namespace impl {
				struct file_size_tag;
			} // namespace impl
		}
	} // namespace streams

	extern template class safe_int<uint64_t, streams::impl::file_size_tag>;

	namespace streams {
		inline namespace v1 { using file_size = caney::safe_int<uint64_t, streams::impl::file_size_tag>; }
	} // namespace streams
} // namespace caney
