#include "format.h"

#include <cassert>
#include <charconv>
#include <limits>
#include <ostream>


std::string _pure format_byte_count(uintmax_t count) {
	static_assert(std::numeric_limits<uintmax_t>::digits == 32 || std::numeric_limits<uintmax_t>::digits == 64);
	if (count < 10000) {
		char buf[6];
		auto [ptr, ec] = std::to_chars(std::begin(buf), std::end(buf), count);
		assert(ec == std::errc { });
		*ptr++ = ' ', *ptr++ = 'B';
		return { buf, ptr };
	}
	char prefix;
	if (count < 10000 * (uintmax_t(1) << 10)) {
		prefix = 'K';
	}
	else if constexpr (std::numeric_limits<uintmax_t>::digits == 32) {
		prefix = 'M';
		count >>= 10;
	}
	else if (count < 10000 * (uintmax_t(1) << 20)) {
		prefix = 'M';
		count >>= 10;
	}
	else if (count < 10000 * (uintmax_t(1) << 30)) {
		prefix = 'G';
		count >>= 20;
	}
	else if (count < 10000 * (uintmax_t(1) << 40)) {
		prefix = 'T';
		count >>= 30;
	}
	else if (count < 10000 * (uintmax_t(1) << 50)) {
		prefix = 'P';
		count >>= 40;
	}
	else {
		prefix = 'E';
		count >>= 50;
	}
	char buf[8];
	auto [ptr, ec] = std::to_chars(std::begin(buf), std::end(buf), count / (uintmax_t(1) << 10));
	assert(ec == std::errc { });
	if (count < 100 * (uintmax_t(1) << 10)) {
		*ptr++ = '.';
		if (count < 10 * (uintmax_t(1) << 10)) {
			count = (count * 100 >> 10) % 100;
			assert(count >= 10); // we never have #.0#
		}
		else {
			count = (count * 10 >> 10) % 10;
		}
		auto [ptr1, ec] = std::to_chars(ptr, std::end(buf), count);
		assert(ec == std::errc { });
		ptr = ptr1;
	}
	*ptr++ = ' ', *ptr++ = prefix, *ptr++ = 'i', *ptr++ = 'B';
	return { buf, ptr };
}

std::ostream & operator<<(std::ostream &os, const struct byte_count &bc) {
	auto formatted = format_byte_count(bc.count);
	auto flags = os.flags();
	if ((flags & std::ios_base::adjustfield) == std::ios_base::internal) {
		os.setf(std::ios_base::right, std::ios_base::adjustfield);
		if (auto width = os.width(); width > 4 && bc.count < 10000) {
			formatted.append(2, os.fill());
		}
	}
	os << formatted;
	os.flags(flags);
	return os;
}
