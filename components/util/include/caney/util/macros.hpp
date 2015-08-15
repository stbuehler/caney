#pragma once

#ifdef __builtin_expect
# define CANEY_LIKELY(x)       __builtin_expect((x),1)
# define CANEY_UNLIKELY(x)     __builtin_expect((x),0)
#else
# define CANEY_LIKELY(x)       (x)
# define CANEY_UNLIKELY(x)     (x)
#endif
