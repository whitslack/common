#pragma once

#include "compiler.h"

#ifdef __APPLE__
#include <machine/endian.h>
#else
#include <endian.h>
#endif

#ifndef BYTE_ORDER
# error "BYTE_ORDER is not defined"
#endif


template <typename T>
struct be_packed {
	static_assert(has_bswap_v<T>, "type parameter must be a byte-swappable type");

	T value_be;

	constexpr be_packed() noexcept(std::is_nothrow_default_constructible_v<T>) = default;
#if BYTE_ORDER == BIG_ENDIAN
	constexpr be_packed(const T &value) noexcept(std::is_nothrow_copy_constructible_v<T>) : value_be(value) { }
	constexpr _pure operator T () const noexcept(std::is_nothrow_copy_constructible_v<T>) { return value_be; }
#elif BYTE_ORDER == LITTLE_ENDIAN
	constexpr be_packed(const T &value) noexcept(noexcept(T { bswap(value) })) : value_be(bswap(value)) { }
	constexpr _pure operator T () const noexcept(noexcept(T { bswap(value_be) })) { return bswap(value_be); }
#endif
	constexpr explicit _pure operator bool () const noexcept(noexcept(bool(value_be))) { return value_be; }
	constexpr bool _pure operator==(const be_packed<T> &rhs) const noexcept(noexcept(bool(value_be == rhs.value_be))) { return value_be == rhs.value_be; }
	constexpr bool _pure operator!=(const be_packed<T> &rhs) const noexcept(noexcept(bool(value_be != rhs.value_be))) { return value_be != rhs.value_be; }

} __attribute__ ((__packed__));

template <typename T>
struct alignas(T) be : be_packed<T> {
	using be_packed<T>::be_packed;
	constexpr be() noexcept = default;
	constexpr be(const be_packed<T> &value) noexcept(std::is_nothrow_copy_constructible_v<be_packed<T>>) : be_packed<T>(value) { }
};


template <typename T>
struct le_packed {
	static_assert(has_bswap_v<T>, "type parameter must be a byte-swappable type");

	T value_le;

	constexpr le_packed() noexcept(std::is_nothrow_default_constructible_v<T>) = default;
#if BYTE_ORDER == BIG_ENDIAN
	constexpr le_packed(const T &value) noexcept(noexcept(T { bswap(value) })) : value_le(bswap(value)) { }
	constexpr _pure operator T () const noexcept(noexcept(T { bswap(value_le) })) { return bswap(value_le); }
#elif BYTE_ORDER == LITTLE_ENDIAN
	constexpr le_packed(const T &value) noexcept(std::is_nothrow_copy_constructible_v<T>) : value_le(value) { }
	constexpr _pure operator T () const noexcept(std::is_nothrow_copy_constructible_v<T>) { return value_le; }
#endif
	constexpr explicit _pure operator bool () const noexcept(noexcept(bool(value_le))) { return value_le; }
	constexpr bool _pure operator==(const le_packed<T> &rhs) const noexcept(noexcept(bool(value_le == rhs.value_le))) { return value_le == rhs.value_le; }
	constexpr bool _pure operator!=(const le_packed<T> &rhs) const noexcept(noexcept(bool(value_le != rhs.value_le))) { return value_le != rhs.value_le; }

} __attribute__ ((__packed__));


template <typename T>
struct alignas(T) le : le_packed<T> {
	using le_packed<T>::le_packed;
	constexpr le() noexcept = default;
	constexpr le(const le_packed<T> &value) noexcept(std::is_nothrow_copy_constructible_v<le_packed<T>>) : le_packed<T>(value) { }
};


template <typename T>
struct be_explicit : public be<T> {
	using be<T>::be;
	operator T () const __attribute__ ((__error__ ("byte-order conversion has no effect")));
};


template <typename T>
struct le_explicit : public le<T> {
	using le<T>::le;
	operator T () const  __attribute__ ((__error__ ("byte-order conversion has no effect")));
};


template <typename T> static constexpr be<T> & _const as_be(T &v) noexcept { return reinterpret_cast<be<T> &>(v); }
template <typename T> static constexpr const be<T> & _const as_be(const T &v) noexcept { return reinterpret_cast<const be<T> &>(v); }
template <typename T> static constexpr be_explicit<T> _pure htobe(const T &v) noexcept(std::is_nothrow_constructible_v<be_explicit<T>, T>) { return v; }
template <typename T> static constexpr T _pure betoh(const be_packed<T> &v) noexcept(std::is_nothrow_constructible_v<T, const be_packed<T> &>) { return v; }

template <typename T> static constexpr le<T> & _const as_le(T &v) noexcept { return reinterpret_cast<le<T> &>(v); }
template <typename T> static constexpr const le<T> & _const as_le(const T &v) noexcept { return reinterpret_cast<const le<T> &>(v); }
template <typename T> static constexpr le_explicit<T> _pure htole(const T &v) noexcept(std::is_nothrow_constructible_v<le_explicit<T>, T>) { return v; }
template <typename T> static constexpr T _pure letoh(const le_packed<T> &v) noexcept(std::is_nothrow_constructible_v<T, const le_packed<T> &>) { return v; }


template <typename T> static constexpr bool _pure operator==(const be_packed<T> &lhs, const T &rhs) noexcept(noexcept(bool(lhs == htobe(rhs)))) { return lhs == htobe(rhs); }
template <typename T> static constexpr bool _pure operator==(const T &lhs, const be_packed<T> &rhs) noexcept(noexcept(bool(htobe(lhs) == rhs))) { return htobe(lhs) == rhs; }
template <typename T> static constexpr bool _pure operator!=(const be_packed<T> &lhs, const T &rhs) noexcept(noexcept(bool(lhs != htobe(rhs)))) { return lhs != htobe(rhs); }
template <typename T> static constexpr bool _pure operator!=(const T &lhs, const be_packed<T> &rhs) noexcept(noexcept(bool(htobe(lhs) != rhs))) { return htobe(lhs) != rhs; }

template <typename T> static constexpr bool _pure operator==(const le_packed<T> &lhs, const T &rhs) noexcept(noexcept(bool(lhs == htole(rhs)))) { return lhs == htole(rhs); }
template <typename T> static constexpr bool _pure operator==(const T &lhs, const le_packed<T> &rhs) noexcept(noexcept(bool(htole(lhs) == rhs))) { return htole(lhs) == rhs; }
template <typename T> static constexpr bool _pure operator!=(const le_packed<T> &lhs, const T &rhs) noexcept(noexcept(bool(lhs != htole(rhs)))) { return lhs != htole(rhs); }
template <typename T> static constexpr bool _pure operator!=(const T &lhs, const le_packed<T> &rhs) noexcept(noexcept(bool(htole(lhs) != rhs))) { return htole(lhs) != rhs; }
