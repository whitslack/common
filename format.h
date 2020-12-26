#pragma once

#include <cstdint>
#include <iosfwd>
#include <string>

#include "compiler.h"


std::string _pure format_byte_count(uintmax_t count);

struct byte_count {
	const uintmax_t count;
	constexpr explicit byte_count(uintmax_t count) noexcept : count(count) { }
	friend std::ostream & operator<<(std::ostream &os, const struct byte_count &bc);
};
