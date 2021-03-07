#include "format.h"

#include <cassert>
#include <charconv>
#include <limits>
#include <ostream>


static std::string _pure format_byte_count(uintmax_t count, char decimal_point, char sign) {
	static_assert(std::numeric_limits<uintmax_t>::digits == 32 || std::numeric_limits<uintmax_t>::digits == 64);
	char buf[9];
	std::to_chars_result res { };
	char *&ptr = res.ptr = buf;
	if (sign) {
		*ptr++ = sign;
	}
	if (count < 10000) {
		res = std::to_chars(ptr, std::end(buf), count);
		assert(res.ec == std::errc { });
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
	res = std::to_chars(ptr, std::end(buf), count / (uintmax_t(1) << 10));
	assert(res.ec == std::errc { });
	if (count < 100 * (uintmax_t(1) << 10)) {
		*ptr++ = decimal_point;
		if (count < 10 * (uintmax_t(1) << 10)) {
			count = (count * 100 >> 10) % 100;
			assert(count >= 10); // we never have #.0#
		}
		else {
			count = (count * 10 >> 10) % 10;
		}
		res = std::to_chars(ptr, std::end(buf), count);
		assert(res.ec == std::errc { });
	}
	*ptr++ = ' ', *ptr++ = prefix, *ptr++ = 'i', *ptr++ = 'B';
	return { buf, ptr };
}

std::string _pure format_byte_count(uintmax_t count, char decimal_point) {
	return format_byte_count(count, decimal_point, '\0');
}

std::string _pure format_byte_count(intmax_t count, char decimal_point) {
	return count < 0 ? format_byte_count(-count, decimal_point, '-') : format_byte_count(count, decimal_point, '\0');
}

static std::ostream & print_byte_count(std::ostream &os, uintmax_t count, char sign) {
	auto formatted = format_byte_count(count, std::use_facet<std::numpunct<char>>(os.getloc()).decimal_point(), sign);
	auto flags = os.flags();
	if ((flags & std::ios_base::adjustfield) == std::ios_base::internal) {
		os.setf(std::ios_base::right, std::ios_base::adjustfield);
		if (count < 10000) {
			formatted.append(2, os.fill());
		}
	}
	os << formatted;
	os.flags(flags);
	return os;
}

std::ostream & operator<<(std::ostream &os, const struct byte_count<true> &bc) {
	return bc.count < 0 ? print_byte_count(os, -bc.count, '-') : print_byte_count(os, bc.count, os.flags() & std::ios_base::showpos ? '+' : '\0');
}

std::ostream & operator<<(std::ostream &os, const struct byte_count<false> &bc) {
	return print_byte_count(os, bc.count, '\0');
}

template struct byte_count<true>;
template struct byte_count<false>;
