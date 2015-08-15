#pragma once

#include <string>
#include <cstdint>
#include <type_traits>
#include <limits>

/* to_string which doesn't depend on any locale */
namespace caney {
	namespace util {
		inline namespace v1 {
			namespace impl {
				std::string to_string(uintmax_t val);
				std::string to_string(intmax_t val);
			}

			template<typename Integral, std::enable_if_t<std::numeric_limits<Integral>::is_integer && std::numeric_limits<Integral>::is_signed>* = nullptr>
			std::string to_string(Integral val)
			{
				return impl::to_string(intmax_t{val});
			}

			template<typename Integral, std::enable_if_t<std::numeric_limits<Integral>::is_integer && !std::numeric_limits<Integral>::is_signed>* = nullptr>
			std::string to_string(Integral val)
			{
				return impl::to_string(uintmax_t{val});
			}
		}
	} // namespace util
} // namespace caney
