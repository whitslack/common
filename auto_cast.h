#pragma once

// copied from GManNickG's blog post at http://www.gmannickg.com/?p=28
// modified to support warning-free narrowing conversions of arithmetic types
// modified to support use in compile-time constant expressions

#include <utility>

template <typename T>
class auto_cast_wrapper
{
	template <typename R>
	friend constexpr auto_cast_wrapper<R> auto_cast(R&& x) noexcept;

private:
	constexpr auto_cast_wrapper(T&& x) noexcept :
	mX(std::forward<T>(x))
	{}

	constexpr auto_cast_wrapper(const auto_cast_wrapper& other) noexcept :
	mX(std::forward<T>(other.mX))
	{}

	auto_cast_wrapper& operator=(const auto_cast_wrapper&) = delete;

	T&& mX;

	template <typename U>
	static constexpr std::enable_if_t<std::is_arithmetic_v<U>, U> do_cast(T&& x) noexcept(noexcept(static_cast<U>(std::forward<T>(x))))
	{
		return static_cast<U>(std::forward<T>(x));
	}

	template <typename U>
	static constexpr std::enable_if_t<!std::is_arithmetic_v<U>, U> do_cast(T&& x) noexcept(noexcept(U{std::forward<T>(x)}))
	{
		// doesn't allow downcasts, otherwise acts like static_cast
		// see: http://stackoverflow.com/questions/5693432/making-auto-cast-safe
		return U{std::forward<T>(x)};
	}

public:
	template <typename U>
	constexpr operator U() const noexcept(noexcept(do_cast<U>(std::forward<T>(mX))))
	{
		return do_cast<U>(std::forward<T>(mX));
	}
};

template <typename R>
constexpr auto_cast_wrapper<R> auto_cast(R&& x) noexcept
{
	return auto_cast_wrapper<R>(std::forward<R>(x));
}
