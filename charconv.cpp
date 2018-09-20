#include "charconv.h"

#include <cstdlib>
#include <string>

#include "compiler.h"


template <typename T, T (*Conv)(const char *, char **)>
static inline std::from_chars_result from_chars(const char *first, const char *last, T &value) {
	std::string str(first, last);
	char *end { };
	errno = 0;
	auto v = Conv(str.data(), &end);
	last = first + (end - str.data());
	if (_unlikely(errno)) {
		return { last, std::errc::result_out_of_range };
	}
	if (_unlikely(!v && last == first)) {
		return { last, std::errc::invalid_argument };
	}
	value = v;
	return { last, { } };
}

template <typename T, int (*Conv)(char *, size_t, const char *, T)>
static inline std::to_chars_result to_chars(char *first, char *last, T value) {
	static constexpr const char fmt[] = { '%', '.', '0' + std::numeric_limits<T>::digits10 / 10, '0' + std::numeric_limits<T>::digits10 % 10, 'g', '\0' };
	auto n = last - first;
	auto r = Conv(first, n, fmt, value);
	if (_unlikely(r > n)) {
		return { last, std::errc::value_too_large };
	}
	return { first + r, { } };
}


namespace std {

std::from_chars_result _weak from_chars(const char *first, const char *last, float &value) { return ::from_chars<float, &::strtof>(first, last, value); }
std::from_chars_result _weak from_chars(const char *first, const char *last, double &value) { return ::from_chars<double, &::strtod>(first, last, value); }
std::from_chars_result _weak from_chars(const char *first, const char *last, long double &value) { return ::from_chars<long double, &::strtold>(first, last, value); }

std::to_chars_result _weak to_chars(char *first, char *last, float value) { return ::to_chars<float, &::strfromf>(first, last, value); }
std::to_chars_result _weak to_chars(char *first, char *last, double value) { return ::to_chars<double, &::strfromd>(first, last, value); }
std::to_chars_result _weak to_chars(char *first, char *last, long double value) { return ::to_chars<long double, &::strfroml>(first, last, value); }

} // namespace std
