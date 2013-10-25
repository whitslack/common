#include "hex.h"

#include <cassert>
#include <ios>


constexpr size_t HexEncoder::input_block_size;
constexpr size_t HexEncoder::output_block_size;

size_t HexEncoder::process(uint8_t (&out)[2], const uint8_t in[], size_t n) {
	static const char encode[16] = {
		'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
	};
	if (n == 0) {
		return 0;
	}
	assert(n == 1);
	out[0] = encode[in[0] >> 4];
	out[1] = encode[in[0] & 0xF];
	return 2;
}


constexpr size_t HexDecoder::input_block_size;
constexpr size_t HexDecoder::output_block_size;

size_t HexDecoder::process(uint8_t (&out)[1], const uint8_t in[], size_t n) {
	static const int8_t decode[55] = {
		 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, -1, -1, -1, -1, -1, -1,
		-1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, 10, 11, 12, 13, 14, 15
	};
	if (n != 2) {
		if (n == 0) {
			return 0;
		}
		throw std::ios_base::failure("invalid hex");
	}
	unsigned int b0, b1;
	if ((b0 = in[0] - '0') > 'f' - '0' || static_cast<int>(b0 = decode[b0]) < 0 || (b1 = in[1] - '0') > 'f' - '0' || static_cast<int>(b1 = decode[b1]) < 0) {
		throw std::ios_base::failure("invalid hex");
	}
	out[0] = static_cast<uint8_t>(b0 << 4 | b1);
	return 1;
}


#include "codec.tcc"

template class CodecSource<HexEncoder>;
template class CodecSource<HexDecoder>;
template class CodecSink<HexEncoder>;
template class CodecSink<HexDecoder>;
