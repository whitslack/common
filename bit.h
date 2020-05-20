#pragma once

#include <bit>

#include "compiler.h"


#if __cpp_lib_int_pow2 < 202002L
namespace std {
	template <typename T> constexpr std::enable_if_t<std::is_integral_v<T> && std::is_unsigned_v<T>, decltype(ispow2(std::declval<T>()))> _const has_single_bit(T x) noexcept { return ispow2(x); }
	template <typename T> constexpr std::enable_if_t<std::is_integral_v<T> && std::is_unsigned_v<T>, decltype(ceil2(std::declval<T>()))> _const bit_ceil(T x) noexcept { return ceil2(x); }
	template <typename T> constexpr std::enable_if_t<std::is_integral_v<T> && std::is_unsigned_v<T>, decltype(floor2(std::declval<T>()))> _const bit_floor(T x) noexcept { return floor2(x); }
	template <typename T> constexpr std::enable_if_t<std::is_integral_v<T> && std::is_unsigned_v<T>, decltype(log2p1(std::declval<T>()))> _const bit_width(T x) noexcept { return log2p1(x); }
}
#endif
