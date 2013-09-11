#include "base64.h"

#include <cassert>
#include <ios>
#include <stdexcept>

#include "codec.h"


constexpr size_t Base64Encoder::input_block_size;
constexpr size_t Base64Encoder::output_block_size;

size_t Base64Encoder::process(uint8_t (&out)[4], const uint8_t in[], size_t n) {
	static const char encode[64] = {
		'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
		'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
		'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
		'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'
	};
	if (n == 0) {
		return 0;
	}
	assert(n <= 3);
	uint8_t b0, b1;
	out[0] = encode[(b0 = in[0]) >> 2];
	if (--n == 0) {
		out[1] = encode[b0 << 4 & (1 << 6) - 1], out[2] = '=', out[3] = '=';
	}
	else {
		out[1] = encode[b0 << 4 & (1 << 6) - 1 | (b1 = in[1]) >> 4];
		if (--n == 0) {
			out[2] = encode[b1 << 2 & (1 << 6) - 1], out[3] = '=';
		}
		else {
			out[2] = encode[b1 << 2 & (1 << 6) - 1 | (b0 = in[2]) >> 6], out[3] = encode[b0 & (1 << 6) - 1];
		}
	}
	return 4;
}


constexpr size_t Base64Decoder::input_block_size;
constexpr size_t Base64Decoder::output_block_size;

size_t Base64Decoder::process(uint8_t (&out)[3], const uint8_t in[], size_t n) {
	static const int8_t decode[80] = {
		62, -1, -1, -1, 63, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1,
		-1, -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17,
		18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1, -1, 26, 27, 28, 29, 30, 31,
		32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51
	};
	if (n != 4) {
		if (n == 0) {
			return 0;
		}
		throw std::ios_base::failure("invalid base64");
	}
	unsigned int b0, b1;
	if ((b0 = in[0] - '+') > 'z' - '+' || static_cast<int>(b0 = decode[b0]) < 0 || (b1 = in[1] - '+') > 'z' - '+' || static_cast<int>(b1 = decode[b1]) < 0) {
		throw std::ios_base::failure("invalid base64");
	}
	out[0] = static_cast<uint8_t>(b0 << 2 | b1 >> 4);
	if ((b0 = in[2]) == '=') {
		if (in[3] != '=') {
			throw std::ios_base::failure("invalid base64");
		}
		return 1;
	}
	if ((b0 -= '+') > 'z' - '+' || static_cast<int>(b0 = decode[b0]) < 0) {
		throw std::ios_base::failure("invalid base64");
	}
	out[1] = static_cast<uint8_t>(b1 << 4 | b0 >> 2);
	if ((b1 = in[3]) == '=') {
		return 2;
	}
	if ((b1 -= '+') > 'z' - '+' || static_cast<int>(b1 = decode[b1]) < 0) {
		throw std::ios_base::failure("invalid base64");
	}
	out[2] = static_cast<uint8_t>(b0 << 6 | b1);
	return 3;
}


std::string encode_base64(const void *in, size_t n_in) {
	std::string out;
	out.reserve((n_in + 2) / 3 * 4);
	StringSink ss(&out);
	CodecSink<Base64Encoder> cs(&ss);
	cs.write_fully(in, n_in);
	cs.finish();
	return out;
}

void decode_base64(void *out, size_t n_out, const char in[], size_t n_in) {
	if (n_in != (n_out + 2) / 3 * 4) {
		throw std::length_error("wrong length");
	}
	MemorySource ms(in, n_in);
	if (CodecSource<Base64Decoder>(&ms).read(out, n_out) != static_cast<ssize_t>(n_out) || ms.avail() != 0) {
		throw std::length_error("wrong length");
	}
}


#include "codec.tcc"

template class CodecSource<Base64Decoder>;
template class CodecSink<Base64Encoder>;
