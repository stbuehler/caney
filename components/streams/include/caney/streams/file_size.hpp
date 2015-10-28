#pragma once

#include "caney/std/safe_int.hpp"

#include "internal.hpp"

__CANEY_STREAMSV1_BEGIN

namespace impl {
	struct file_size_tag;
} // namespace impl

__CANEY_STREAMSV1_END

extern template class caney::safe_int<uint64_t, caney::streams::impl::file_size_tag>;

__CANEY_STREAMSV1_BEGIN

/** @brief dedicated file size type */
using file_size = caney::safe_int<uint64_t, streams::impl::file_size_tag>;

__CANEY_STREAMSV1_END
