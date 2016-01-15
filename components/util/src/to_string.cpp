#include "caney/util/to_string.hpp"

namespace caney {
	namespace util {
		inline namespace v1 {
			namespace impl {
				std::string to_string(uintmax_t val) {
					if (0 == val) return std::string("0");
					size_t len = 0;
					for (uintmax_t i = val; i > 0; i /= 10, ++len)
						;
					std::string result(len, '\0');
					for (uintmax_t i = val; i > 0; i /= 10) {
						--len;
						result[len] = '0' + (i % 10);
					}

					return result;
				}

				std::string to_string(intmax_t val) {
					if (0 == val) return std::string("0");
					size_t len = (val < 0) ? 1 : 0;
					uintmax_t const uval = static_cast<uintmax_t>(std::abs(val));
					for (uintmax_t i = uval; i > 0; i /= 10, ++len)
						;
					std::string result(len, '\0');
					result[0] = '-'; // positive numbers will overwrite this
					for (uintmax_t i = uval; i > 0; i /= 10) {
						--len;
						result[len] = '0' + (i % 10);
					}

					return result;
				}
			} // namespace impl
		}
	} // namespace util
} // namespace caney
