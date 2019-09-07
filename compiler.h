#pragma once

#include <cstdint>
#include <type_traits>

#if __cpp_attributes
#	if __has_cpp_attribute(deprecated)
#		define _deprecated [[deprecated]]
#	elif __GNUC__
#		define _deprecated __attribute__ ((__deprecated__))
#	else
#		define _deprecated
#	endif
#	if __has_cpp_attribute(fallthrough)
#		define _fallthrough [[fallthrough]]
#	else
#		define _fallthrough
#	endif
#	if __has_cpp_attribute(nodiscard)
#		define _nodiscard [[nodiscard]]
#	elif __GNUC__
#		define _nodiscard __attribute__ ((__warn_unused_result__))
#	else
#		define _nodiscard
#	endif
#	if __has_cpp_attribute(noreturn)
#		define _noreturn [[noreturn]]
#	elif __GNUC__
#		define _noreturn __attribute__ ((__noreturn__))
#	else
#		define _noreturn
#	endif
#	if __has_cpp_attribute(maybe_unused)
#		define _unused [[maybe_unused]]
#	elif __GNUC__
#		define _unused __attribute__ ((__unused__))
#	else
#		define _unused
#	endif
#else
#	define _deprecated
#	define _fallthrough
#	define _nodiscard
#	define _noreturn
#	define _unused
#endif

#ifdef __GNUC__

#define _always_inline __attribute__ ((__always_inline__))
#define _const __attribute__ ((__const__))
#define _hidden __attribute__ ((__visibility__ ("hidden")))
#define _nonnull(...) __attribute__ ((__nonnull__ (__VA_ARGS__)))
#define _pure __attribute__ ((__pure__))
#define _visible __attribute__ ((__visibility__ ("default")))
#define _weak __attribute__ ((__weak__))
#define _weakref(...) __attribute__ ((__weakref__ (__VA_ARGS__)))

#define _restrict __restrict

#define _likely(...) __builtin_expect(!!(__VA_ARGS__), true)
#define _unlikely(...) __builtin_expect(!!(__VA_ARGS__), false)

static constexpr auto _const _parity(unsigned v) noexcept { return __builtin_parity(v); }
static constexpr auto _const _parity(unsigned long v) noexcept { return __builtin_parityl(v); }
static constexpr auto _const _parity(unsigned long long v) noexcept { return __builtin_parityll(v); }

template <typename T> static constexpr std::enable_if_t<std::is_integral_v<T> && sizeof(T) == sizeof(uint16_t), T> _const bswap(T v) noexcept { return __builtin_bswap16(v); }
template <typename T> static constexpr std::enable_if_t<std::is_integral_v<T> && sizeof(T) == sizeof(uint32_t), T> _const bswap(T v) noexcept { return __builtin_bswap32(v); }
template <typename T> static constexpr std::enable_if_t<std::is_integral_v<T> && sizeof(T) == sizeof(uint64_t), T> _const bswap(T v) noexcept { return __builtin_bswap64(v); }

#ifdef __SIZEOF_INT128__
static constexpr unsigned __int128 _const __bswap128(unsigned __int128 v) noexcept {
	union {
		uint64_t q[2];
		unsigned __int128 o;
	} u { .o = v };
	uint64_t t = __builtin_bswap64(u.q[0]);
	u.q[0] = __builtin_bswap64(u.q[1]);
	u.q[1] = t;
	return u.o;
}
static constexpr signed __int128 _const bswap(signed __int128 v) noexcept { return __bswap128(v); }
static constexpr unsigned __int128 _const bswap(unsigned __int128 v) noexcept { return __bswap128(v); }
#endif

#define __USER_LABEL_PREFIX_STR__ __USER_LABEL_PREFIX_STR_0(__USER_LABEL_PREFIX__)
#define __USER_LABEL_PREFIX_STR_0(prefix) __USER_LABEL_PREFIX_STR_1(prefix)
#define __USER_LABEL_PREFIX_STR_1(prefix) #prefix

#else

#define _const
#define _hidden
#define _nonnull(...)
#define _pure
#define _visible

#define _restrict

#define _likely(...) (__VA_ARGS__)
#define _unlikely(...) (__VA_ARGS__)

#endif

template <typename T> static constexpr std::enable_if_t<std::is_enum_v<T>, T> _const bswap(T v) noexcept { return static_cast<T>(bswap(static_cast<std::underlying_type_t<T>>(v))); }

template <typename T, typename Enable = void>
struct has_bswap : std::false_type { };

template <typename T>
struct has_bswap<T, std::enable_if_t<std::is_same_v<decltype(bswap(std::declval<T>())), std::remove_reference_t<T>>>> : std::true_type { };

template <typename T>
constexpr bool has_bswap_v = has_bswap<T>::value;
