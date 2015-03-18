#pragma once

#include <climits>
#include <cstdint>
#include <type_traits>

#ifdef __GNUC__

#define _const __attribute__ ((__const__))
#define _nonnull(...) __attribute__ ((__nonnull__ (__VA_ARGS__)))
#define _noreturn __attribute__ ((__noreturn__))
#define _pure __attribute__ ((__pure__))
#define _unused __attribute__ ((__unused__))
#define _weak __attribute__ ((__weak__))
#define _wur __attribute__ ((__warn_usused_result__))

#define _restrict __restrict

static inline int _ffs(int v) { return __builtin_ffs(v); }
static inline int _ffs(long v) { return __builtin_ffsl(v); }
static inline int _ffs(long long v) { return __builtin_ffsll(v); }

static inline int _clz(unsigned v) { return __builtin_clz(v); }
static inline int _clz(unsigned long v) { return __builtin_clzl(v); }
static inline int _clz(unsigned long long v) { return __builtin_clzll(v); }

static inline int _ctz(unsigned v) { return __builtin_ctz(v); }
static inline int _ctz(unsigned long v) { return __builtin_ctzl(v); }
static inline int _ctz(unsigned long long v) { return __builtin_ctzll(v); }

static inline int _clrsb(int v) { return __builtin_clrsb(v); }
static inline int _clrsb(long v) { return __builtin_clrsbl(v); }
static inline int _clrsb(long long v) { return __builtin_clrsbll(v); }

static inline int _popcount(unsigned v) { return __builtin_popcount(v); }
static inline int _popcount(unsigned long v) { return __builtin_popcountl(v); }
static inline int _popcount(unsigned long long v) { return __builtin_popcountll(v); }

static inline int _parity(unsigned v) { return __builtin_parity(v); }
static inline int _parity(unsigned long v) { return __builtin_parityl(v); }
static inline int _parity(unsigned long long v) { return __builtin_parityll(v); }

static inline unsigned rotl(unsigned v, unsigned s) { return v << s | v >> sizeof v * 8 - s; }
static inline unsigned long rotl(unsigned long v, unsigned s) { return v << s | v >> sizeof v * 8 - s; }
static inline unsigned long long rotl(unsigned long long v, unsigned s) { return v << s | v >> sizeof v * 8 - s; }

static inline unsigned rotr(unsigned v, unsigned s) { return v >> s | v << sizeof v * 8 - s; }
static inline unsigned long rotr(unsigned long v, unsigned s) { return v >> s | v << sizeof v * 8 - s; }
static inline unsigned long long rotr(unsigned long long v, unsigned s) { return v >> s | v << sizeof v * 8 - s; }

static inline int16_t bswap(int16_t v) { return __builtin_bswap16(v); }
static inline uint16_t bswap(uint16_t v) { return __builtin_bswap16(v); }
static inline int32_t bswap(int32_t v) { return __builtin_bswap32(v); }
static inline uint32_t bswap(uint32_t v) { return __builtin_bswap32(v); }
static inline int64_t bswap(int64_t v) { return __builtin_bswap64(v); }
static inline uint64_t bswap(uint64_t v) { return __builtin_bswap64(v); }

#if ULONG_MAX == UINT32_MAX && UINT32_MAX == UINT_MAX
static inline long bswap(long v) { return __builtin_bswap32(v); }
static inline unsigned long bswap(unsigned long v) { return __builtin_bswap32(v); }
#elif ULONG_LONG_MAX == UINT64_MAX && UINT64_MAX == ULONG_MAX
static inline long long bswap(long long v) { return __builtin_bswap64(v); }
static inline unsigned long long bswap(unsigned long long v) { return __builtin_bswap64(v); }
#endif

#if BYTE_ORDER == BIG_ENDIAN
template <typename T> static inline typename std::enable_if<std::is_integral<T>::value, T>::type htobe(T v) { return v; }
template <typename T> static inline typename std::enable_if<std::is_integral<T>::value, T>::type betoh(T v) { return v; }
template <typename T> static inline typename std::enable_if<std::is_integral<T>::value, T>::type htole(T v) { return bswap(v); }
template <typename T> static inline typename std::enable_if<std::is_integral<T>::value, T>::type letoh(T v) { return bswap(v); }
#elif BYTE_ORDER == LITTLE_ENDIAN
template <typename T> static inline typename std::enable_if<std::is_integral<T>::value, T>::type htobe(T v) { return bswap(v); }
template <typename T> static inline typename std::enable_if<std::is_integral<T>::value, T>::type betoh(T v) { return bswap(v); }
template <typename T> static inline typename std::enable_if<std::is_integral<T>::value, T>::type htole(T v) { return v; }
template <typename T> static inline typename std::enable_if<std::is_integral<T>::value, T>::type letoh(T v) { return v; }
#endif

#ifdef __SIZEOF_INT128__
static inline unsigned __int128 __bswap128(unsigned __int128 v) {
	union {
		uint64_t q[2];
		unsigned __int128 o;
	} u;
	u.o = v;
	uint64_t t = __builtin_bswap64(u.q[0]);
	u.q[0] = __builtin_bswap64(u.q[1]);
	u.q[1] = t;
	return u.o;
}
static inline signed __int128 bswap(signed __int128 v) { return __bswap128(v); }
static inline unsigned __int128 bswap(unsigned __int128 v) { return __bswap128(v); }
#if BYTE_ORDER == BIG_ENDIAN
static inline signed __int128 htobe(signed __int128 v) { return v; }
static inline signed __int128 betoh(signed __int128 v) { return v; }
static inline signed __int128 htole(signed __int128 v) { return bswap(v); }
static inline signed __int128 letoh(signed __int128 v) { return bswap(v); }
static inline unsigned __int128 htobe(unsigned __int128 v) { return v; }
static inline unsigned __int128 betoh(unsigned __int128 v) { return v; }
static inline unsigned __int128 htole(unsigned __int128 v) { return bswap(v); }
static inline unsigned __int128 letoh(unsigned __int128 v) { return bswap(v); }
#elif BYTE_ORDER == LITTLE_ENDIAN
static inline signed __int128 htobe(signed __int128 v) { return bswap(v); }
static inline signed __int128 betoh(signed __int128 v) { return bswap(v); }
static inline signed __int128 htole(signed __int128 v) { return v; }
static inline signed __int128 letoh(signed __int128 v) { return v; }
static inline unsigned __int128 htobe(unsigned __int128 v) { return bswap(v); }
static inline unsigned __int128 betoh(unsigned __int128 v) { return bswap(v); }
static inline unsigned __int128 htole(unsigned __int128 v) { return v; }
static inline unsigned __int128 letoh(unsigned __int128 v) { return v; }
#endif
#endif

#else

#define _const
#define _nonnull(...)
#define _noreturn
#define _pure
#define _unused
#define _wur

#define _restrict

#endif

template <typename T> static inline typename std::enable_if<std::is_enum<T>::value, T>::type bswap(T v) { return static_cast<T>(bswap(static_cast<typename std::underlying_type<T>::type>(v))); }
template <typename T> static inline typename std::enable_if<std::is_enum<T>::value, T>::type htobe(T v) { return static_cast<T>(htobe(static_cast<typename std::underlying_type<T>::type>(v))); }
template <typename T> static inline typename std::enable_if<std::is_enum<T>::value, T>::type betoh(T v) { return static_cast<T>(betoh(static_cast<typename std::underlying_type<T>::type>(v))); }
template <typename T> static inline typename std::enable_if<std::is_enum<T>::value, T>::type htole(T v) { return static_cast<T>(htole(static_cast<typename std::underlying_type<T>::type>(v))); }
template <typename T> static inline typename std::enable_if<std::is_enum<T>::value, T>::type letoh(T v) { return static_cast<T>(letoh(static_cast<typename std::underlying_type<T>::type>(v))); }
