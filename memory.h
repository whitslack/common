#pragma once

#include <cstddef>

#if __cplusplus < 201700L
namespace std {
	template <typename C> constexpr auto size(const C &c) noexcept(noexcept(c.size())) -> decltype(c.size()) { return c.size(); }
	template <typename T, size_t N> constexpr size_t size(const T (&)[N]) noexcept { return N; }
}
#endif
