#pragma once

#include <cstdint>
#include <iosfwd>
#include <locale>
#include <string>

#include "compiler.h"


std::string _pure format_byte_count(uintmax_t count, char decimal_point = std::use_facet<std::numpunct<char>>(std::locale()).decimal_point());
std::string _pure format_byte_count(intmax_t count, char decimal_point = std::use_facet<std::numpunct<char>>(std::locale()).decimal_point());

template <bool Signed>
struct byte_count {
	using count_t = std::conditional_t<Signed, intmax_t, uintmax_t>;
	const count_t count;
	constexpr explicit byte_count(count_t count) noexcept : count(count) { }
	friend std::ostream & operator<<(std::ostream &os, const struct byte_count<true> &bc);
	friend std::ostream & operator<<(std::ostream &os, const struct byte_count<false> &bc);
};

explicit byte_count(intmax_t) -> byte_count<true>;
explicit byte_count(uintmax_t) -> byte_count<false>;

extern template struct byte_count<true>;
extern template struct byte_count<false>;
