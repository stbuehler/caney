#include "caney/util/simple_io.hpp"
#include "caney/std/error_code.hpp"

#include <unistd.h>

namespace caney {
	namespace util {
		inline namespace v1 {
			std::size_t write_all(int fd, void const* buf, std::size_t len, std::error_code& ec) {
				std::size_t written = 0;
				ec.clear();

				while (len > 0) {
					::ssize_t result = ::write(fd, buf, len);
					if (-1 == result) {
						switch (errno) {
						case EINTR:
							continue; // try again immediately
						default:
							ec = std::error_code(errno, std::generic_category());
							return written;
						}
					} else {
						written += std::size_t(result);
						len -= std::size_t(result);
						buf = reinterpret_cast<char const*>(buf) + std::size_t(result);
					}
				}

				return written;
			}
		}
	} // namespace util
} // namespace caney
