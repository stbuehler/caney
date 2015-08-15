#pragma once

#include <system_error>

namespace caney {
	namespace util {
		inline namespace v1 {
			// writes as many bytes as possible, returning the number of bytes written.
			// iff not all bytes could be written ec will contain the reason
			std::size_t write_all(int fd, void const* buf, std::size_t len, std::error_code& ec);
		}
	} // namespace util
} // namespace caney
