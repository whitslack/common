#pragma once

#include <cstdint>
#include <iosfwd>
#include <locale>
#include <string>

#include "compiler.h"


std::string _pure format_byte_count(uintmax_t count, char decimal_point = std::use_facet<std::numpunct<char>>(std::locale()).decimal_point());

struct byte_count {
	const uintmax_t count;
	constexpr explicit byte_count(uintmax_t count) noexcept : count(count) { }
	friend std::ostream & operator<<(std::ostream &os, const struct byte_count &bc);
};
