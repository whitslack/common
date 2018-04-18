#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>

#ifndef __cpp_lib_nonmember_container_access
namespace std {
	template <typename C> constexpr auto size(const C &c) noexcept(noexcept(c.size())) -> decltype(c.size()) { return c.size(); }
	template <typename T, size_t N> constexpr size_t size(const T (&)[N]) noexcept { return N; }
}
#endif

static inline std::unique_ptr<uint8_t[]> make_buffer(size_t size) {
	return std::unique_ptr<uint8_t[]>(new uint8_t[size]);
}
