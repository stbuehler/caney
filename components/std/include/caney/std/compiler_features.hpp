#pragma once

#if __cpp_constexpr >= 201304
#	define CANEY_RELAXED_CONSTEXPR constexpr
#else
#	define CANEY_RELAXED_CONSTEXPR
#endif
