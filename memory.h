#pragma once

#include <cstdlib>
#include <memory>

#ifndef __cpp_lib_nonmember_container_access
namespace std {
	template <typename C> constexpr auto size(const C &c) noexcept(noexcept(c.size())) -> decltype(c.size()) { return c.size(); }
	template <typename T, size_t N> constexpr size_t size(const T (&)[N]) noexcept { return N; }
}
#endif


namespace polyfill {
	template <typename T> inline std::enable_if_t<!std::is_array_v<T>, std::unique_ptr<T>> make_unique_for_overwrite() { return std::unique_ptr<T>(new T); }
	template <typename T> inline std::enable_if_t<std::is_unbounded_array_v<T>, std::unique_ptr<T>> make_unique_for_overwrite(size_t n) { return std::unique_ptr<T>(new std::remove_extent_t<T>[n]); }
	template <typename T, typename... Args> inline std::enable_if_t<std::is_bounded_array_v<T>, std::unique_ptr<T>> make_unique_for_overwrite(Args &&...args) = delete;
}
namespace std {
	using namespace polyfill;
}


template <typename T>
using unique_c_ptr = std::unique_ptr<T, std::integral_constant<void (*)(void *), &std::free>>;
