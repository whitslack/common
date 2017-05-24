#pragma once

#include <climits>
#include <cstdint>
#include <type_traits>

#ifdef __GNUC__

#define _always_inline __attribute__ ((__always_inline__))
#define _const __attribute__ ((__const__))
#define _hidden __attribute__ ((__visibility__ ("hidden")))
#define _nonnull(...) __attribute__ ((__nonnull__ (__VA_ARGS__)))
#define _noreturn __attribute__ ((__noreturn__))
#define _pure __attribute__ ((__pure__))
#define _unused __attribute__ ((__unused__))
#define _visible __attribute__ ((__visibility__ ("default")))
#define _weak __attribute__ ((__weak__))
#define _weakref(...) __attribute__ ((__weakref__ (__VA_ARGS__)))
#define _wur __attribute__ ((__warn_usused_result__))

#if __GNUC__ >= 7
#	if __cplusplus >= 201500L
#		define _fallthrough [[fallthrough]]
#	else
#		define _fallthrough [[gnu::fallthrough]]
#	endif
#else
#	define _fallthrough
#endif

#define _restrict __restrict

static constexpr auto _const _ffs(int v) { return __builtin_ffs(v); }
static constexpr auto _const _ffs(long v) { return __builtin_ffsl(v); }
static constexpr auto _const _ffs(long long v) { return __builtin_ffsll(v); }

static constexpr auto _const _clz(unsigned v) { return __builtin_clz(v); }
static constexpr auto _const _clz(unsigned long v) { return __builtin_clzl(v); }
static constexpr auto _const _clz(unsigned long long v) { return __builtin_clzll(v); }

static constexpr auto _const _ctz(unsigned v) { return __builtin_ctz(v); }
static constexpr auto _const _ctz(unsigned long v) { return __builtin_ctzl(v); }
static constexpr auto _const _ctz(unsigned long long v) { return __builtin_ctzll(v); }

static constexpr auto _const _clrsb(int v) { return __builtin_clrsb(v); }
static constexpr auto _const _clrsb(long v) { return __builtin_clrsbl(v); }
static constexpr auto _const _clrsb(long long v) { return __builtin_clrsbll(v); }

static constexpr auto _const _popcount(unsigned v) { return __builtin_popcount(v); }
static constexpr auto _const _popcount(unsigned long v) { return __builtin_popcountl(v); }
static constexpr auto _const _popcount(unsigned long long v) { return __builtin_popcountll(v); }

static constexpr auto _const _parity(unsigned v) { return __builtin_parity(v); }
static constexpr auto _const _parity(unsigned long v) { return __builtin_parityl(v); }
static constexpr auto _const _parity(unsigned long long v) { return __builtin_parityll(v); }

static constexpr unsigned _const rotl(unsigned v, unsigned s) { return v << s | v >> sizeof v * CHAR_BIT - s; }
static constexpr unsigned long _const rotl(unsigned long v, unsigned s) { return v << s | v >> sizeof v * CHAR_BIT - s; }
static constexpr unsigned long long _const rotl(unsigned long long v, unsigned s) { return v << s | v >> sizeof v * CHAR_BIT - s; }

static constexpr unsigned _const rotr(unsigned v, unsigned s) { return v >> s | v << sizeof v * CHAR_BIT - s; }
static constexpr unsigned long _const rotr(unsigned long v, unsigned s) { return v >> s | v << sizeof v * CHAR_BIT - s; }
static constexpr unsigned long long _const rotr(unsigned long long v, unsigned s) { return v >> s | v << sizeof v * CHAR_BIT - s; }

template <typename T> static constexpr std::enable_if_t<std::is_integral<T>::value && sizeof(T) == sizeof(uint16_t), T> _const bswap(T v) { return __builtin_bswap16(v); }
template <typename T> static constexpr std::enable_if_t<std::is_integral<T>::value && sizeof(T) == sizeof(uint32_t), T> _const bswap(T v) { return __builtin_bswap32(v); }
template <typename T> static constexpr std::enable_if_t<std::is_integral<T>::value && sizeof(T) == sizeof(uint64_t), T> _const bswap(T v) { return __builtin_bswap64(v); }

#ifdef __SIZEOF_INT128__
static constexpr unsigned __int128 _const __bswap128(unsigned __int128 v) {
	union {
		uint64_t q[2];
		unsigned __int128 o;
	} u { .o = v };
	uint64_t t = __builtin_bswap64(u.q[0]);
	u.q[0] = __builtin_bswap64(u.q[1]);
	u.q[1] = t;
	return u.o;
}
static constexpr signed __int128 _const bswap(signed __int128 v) { return __bswap128(v); }
static constexpr unsigned __int128 _const bswap(unsigned __int128 v) { return __bswap128(v); }
#endif

#define __USER_LABEL_PREFIX_STR__ __USER_LABEL_PREFIX_STR_0(__USER_LABEL_PREFIX__)
#define __USER_LABEL_PREFIX_STR_0(prefix) __USER_LABEL_PREFIX_STR_1(prefix)
#define __USER_LABEL_PREFIX_STR_1(prefix) #prefix

#else

#define _const
#define _hidden
#define _nonnull(...)
#define _noreturn
#define _pure
#define _unused
#define _visible
#define _wur

#define _fallthrough

#define _restrict

#endif

template <typename T> static constexpr std::enable_if_t<std::is_enum<T>::value, T> _const bswap(T v) { return static_cast<T>(bswap(static_cast<std::underlying_type_t<T>>(v))); }

template <typename T, typename Enable = void>
struct has_bswap : std::false_type { };

template <typename T>
struct has_bswap<T, std::enable_if_t<std::is_same<decltype(bswap(std::declval<T>())), std::remove_reference_t<T>>::value>> : std::true_type { };
