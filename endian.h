#pragma once

#include <utility>

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
struct be {
	static_assert(has_bswap_v<T>, "type parameter must be a byte-swappable type");

	T value_be;

	constexpr be() noexcept(std::is_nothrow_default_constructible_v<T>) = default;
#if BYTE_ORDER == BIG_ENDIAN
	constexpr be(const T &value) noexcept(std::is_nothrow_copy_constructible_v<T>) : value_be(value) { }
	constexpr _pure operator T () const noexcept(std::is_nothrow_copy_constructible_v<T>) { return value_be; }
#elif BYTE_ORDER == LITTLE_ENDIAN
	constexpr be(const T &value) noexcept(noexcept(T(bswap(value)))) : value_be(bswap(value)) { }
	constexpr _pure operator T () const noexcept(noexcept(T(bswap(value_be)))) { return bswap(value_be); }
#endif
	constexpr explicit _pure operator bool () const noexcept(noexcept(bool(value_be))) { return value_be; }
	constexpr bool _pure operator!() const noexcept(noexcept(bool(!value_be))) { return !value_be; }
	constexpr bool _pure operator==(const be &rhs) const noexcept(noexcept(bool(value_be == rhs.value_be))) { return value_be == rhs.value_be; }
	constexpr decltype(auto) operator&=(const be &rhs) noexcept(noexcept(value_be &= rhs.value_be)) { return value_be &= rhs.value_be, *this; }
	constexpr decltype(auto) operator|=(const be &rhs) noexcept(noexcept(value_be |= rhs.value_be)) { return value_be |= rhs.value_be, *this; }
	constexpr decltype(auto) operator^=(const be &rhs) noexcept(noexcept(value_be ^= rhs.value_be)) { return value_be ^= rhs.value_be, *this; }
	template <typename R> constexpr auto _pure operator&(R &&rhs) const noexcept(noexcept(be(*this) &= std::forward<R>(rhs))) -> std::enable_if_t<std::is_same_v<T, decltype(value_be & std::forward<R>(rhs))>, be> { return be(*this) &= std::forward<R>(rhs); }
	template <typename R> constexpr auto _pure operator|(R &&rhs) const noexcept(noexcept(be(*this) |= std::forward<R>(rhs))) -> std::enable_if_t<std::is_same_v<T, decltype(value_be | std::forward<R>(rhs))>, be> { return be(*this) |= std::forward<R>(rhs); }
	template <typename R> constexpr auto _pure operator^(R &&rhs) const noexcept(noexcept(be(*this) ^= std::forward<R>(rhs))) -> std::enable_if_t<std::is_same_v<T, decltype(value_be ^ std::forward<R>(rhs))>, be> { return be(*this) ^= std::forward<R>(rhs); }

};


template <typename T>
struct le {
	static_assert(has_bswap_v<T>, "type parameter must be a byte-swappable type");

	T value_le;

	constexpr le() noexcept(std::is_nothrow_default_constructible_v<T>) = default;
#if BYTE_ORDER == BIG_ENDIAN
	constexpr le(const T &value) noexcept(noexcept(T(bswap(value)))) : value_le(bswap(value)) { }
	constexpr _pure operator T () const noexcept(noexcept(T(bswap(value_le)))) { return bswap(value_le); }
#elif BYTE_ORDER == LITTLE_ENDIAN
	constexpr le(const T &value) noexcept(std::is_nothrow_copy_constructible_v<T>) : value_le(value) { }
	constexpr _pure operator T () const noexcept(std::is_nothrow_copy_constructible_v<T>) { return value_le; }
#endif
	constexpr explicit _pure operator bool () const noexcept(noexcept(bool(value_le))) { return value_le; }
	constexpr bool _pure operator!() const noexcept(noexcept(bool(!value_le))) { return !value_le; }
	constexpr bool _pure operator==(const le &rhs) const noexcept(noexcept(bool(value_le == rhs.value_le))) { return value_le == rhs.value_le; }
	constexpr decltype(auto) operator&=(const le &rhs) noexcept(noexcept(value_le &= rhs.value_le)) { return value_le &= rhs.value_le, *this; }
	constexpr decltype(auto) operator|=(const le &rhs) noexcept(noexcept(value_le |= rhs.value_le)) { return value_le |= rhs.value_le, *this; }
	constexpr decltype(auto) operator^=(const le &rhs) noexcept(noexcept(value_le ^= rhs.value_le)) { return value_le ^= rhs.value_le, *this; }
	template <typename R> constexpr auto _pure operator&(R &&rhs) const noexcept(noexcept(le(*this) &= std::forward<R>(rhs))) -> std::enable_if_t<std::is_same_v<T, decltype(value_le & std::forward<R>(rhs))>, le> { return le(*this) &= std::forward<R>(rhs); }
	template <typename R> constexpr auto _pure operator|(R &&rhs) const noexcept(noexcept(le(*this) |= std::forward<R>(rhs))) -> std::enable_if_t<std::is_same_v<T, decltype(value_le | std::forward<R>(rhs))>, le> { return le(*this) |= std::forward<R>(rhs); }
	template <typename R> constexpr auto _pure operator^(R &&rhs) const noexcept(noexcept(le(*this) ^= std::forward<R>(rhs))) -> std::enable_if_t<std::is_same_v<T, decltype(value_le ^ std::forward<R>(rhs))>, le> { return le(*this) ^= std::forward<R>(rhs); }

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
template <typename T> static constexpr T _pure betoh(const be<T> &v) noexcept(std::is_nothrow_constructible_v<T, const be<T> &>) { return v; }

template <typename T> static constexpr le<T> & _const as_le(T &v) noexcept { return reinterpret_cast<le<T> &>(v); }
template <typename T> static constexpr const le<T> & _const as_le(const T &v) noexcept { return reinterpret_cast<const le<T> &>(v); }
template <typename T> static constexpr le_explicit<T> _pure htole(const T &v) noexcept(std::is_nothrow_constructible_v<le_explicit<T>, T>) { return v; }
template <typename T> static constexpr T _pure letoh(const le<T> &v) noexcept(std::is_nothrow_constructible_v<T, const le<T> &>) { return v; }


template <typename T> static constexpr bool _pure operator==(const be<T> &lhs, const T &rhs) noexcept(noexcept(bool(lhs == htobe(rhs)))) { return lhs == htobe(rhs); }
template <typename T> static constexpr bool _pure operator==(const T &lhs, const be<T> &rhs) noexcept(noexcept(bool(htobe(lhs) == rhs))) { return htobe(lhs) == rhs; }
template <typename T> static constexpr bool _pure operator!=(const be<T> &lhs, const T &rhs) noexcept(noexcept(bool(lhs != htobe(rhs)))) { return lhs != htobe(rhs); }
template <typename T> static constexpr bool _pure operator!=(const T &lhs, const be<T> &rhs) noexcept(noexcept(bool(htobe(lhs) != rhs))) { return htobe(lhs) != rhs; }

template <typename T> static constexpr bool _pure operator==(const le<T> &lhs, const T &rhs) noexcept(noexcept(bool(lhs == htole(rhs)))) { return lhs == htole(rhs); }
template <typename T> static constexpr bool _pure operator==(const T &lhs, const le<T> &rhs) noexcept(noexcept(bool(htole(lhs) == rhs))) { return htole(lhs) == rhs; }
template <typename T> static constexpr bool _pure operator!=(const le<T> &lhs, const T &rhs) noexcept(noexcept(bool(lhs != htole(rhs)))) { return lhs != htole(rhs); }
template <typename T> static constexpr bool _pure operator!=(const T &lhs, const le<T> &rhs) noexcept(noexcept(bool(htole(lhs) != rhs))) { return htole(lhs) != rhs; }
