#include "caney/std/error_code.hpp"

#include <unistd.h>

namespace caney {
	inline namespace stdv1 {
		std::error_code errno_error_code() {
			return std::error_code(errno, std::generic_category());
		}
	}
}
