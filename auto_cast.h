// copied from GManNickG's blog post at http://www.gmannickg.com/?p=28
// modified to support warning-free narrowing conversions of arithmetic types

#include <utility>

template <typename T>
class auto_cast_wrapper
{
public:
	template <typename R>
	friend auto_cast_wrapper<R> auto_cast(R&& x);

	template <typename U>
	operator U() const
	{
		return do_cast<U>(std::forward<T>(mX));
	}

private:
	auto_cast_wrapper(T&& x) :
	mX(std::forward<T>(x))
	{}

	auto_cast_wrapper(const auto_cast_wrapper& other) :
	mX(std::forward<T>(other.mX))
	{}

	auto_cast_wrapper& operator=(const auto_cast_wrapper&) = delete;

	T&& mX;

	template <typename U>
	static typename std::enable_if<std::is_arithmetic<U>::value, U>::type do_cast(T&& x)
	{
		return static_cast<U>(std::forward<T>(x));
	}

	template <typename U>
	static typename std::enable_if<!std::is_arithmetic<U>::value, U>::type do_cast(T&& x)
	{
		// doesn't allow downcasts, otherwise acts like static_cast
		// see: http://stackoverflow.com/questions/5693432/making-auto-cast-safe
		return U{std::forward<T>(x)};
	}
};

template <typename R>
auto_cast_wrapper<R> auto_cast(R&& x)
{
	return auto_cast_wrapper<R>(std::forward<R>(x));
}
