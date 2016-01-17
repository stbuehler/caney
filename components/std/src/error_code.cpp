#include "caney/std/error_code.hpp"

#include <unistd.h>

__CANEY_STDV1_BEGIN

std::error_code errno_error_code() {
	return std::error_code(errno, std::generic_category());
}

std::error_code errno_error_code(int user_errno) {
	return std::error_code(user_errno, std::generic_category());
}

__CANEY_STDV1_END
