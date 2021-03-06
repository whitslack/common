#pragma once

#include <charconv>


namespace polyfill {

std::from_chars_result from_chars(const char *first, const char *last, float &value);
std::from_chars_result from_chars(const char *first, const char *last, double &value);
std::from_chars_result from_chars(const char *first, const char *last, long double &value);

std::to_chars_result to_chars(char *first, char *last, float value);
std::to_chars_result to_chars(char *first, char *last, double value);
std::to_chars_result to_chars(char *first, char *last, long double value);

} // namespace polyfill

namespace std {
	using namespace polyfill;
}
