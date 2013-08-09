#pragma once

#ifdef __GNUC__

#define _const __attribute__ ((__const__))
#define _nonnull(...) __attribute__ ((__nonnull__ (__VA_ARGS__)))
#define _pure __attribute__ ((__pure__))
#define _unused __attribute__ ((__unused__))
#define _wur __attribute__ ((__warn_usused_result__))

#else

#define _const
#define _nonnull(...)
#define _pure
#define _unused
#define _wur

#endif
