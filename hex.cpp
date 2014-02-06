#include "hex.h"

#include <cassert>
#include <ios>


bool HexEncoder::process(uint8_t *&out, size_t n_out, const uint8_t *&in, size_t n_in) {
	static constexpr char encode[16] = {
		'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
	};
	if (buf_full) {
		if (n_out == 0) {
			return false;
		}
		*out++ = encode[buf & 0xF], buf_full = false, --n_out;
	}
	while (n_in > 0) {
		if (n_out == 0) {
			return false;
		}
		uint b = *in++; --n_in;
		*out++ = encode[b >> 4], --n_out;
		if (n_out == 0) {
			buf = static_cast<uint8_t>(b), buf_full = true;
			return false;
		}
		*out++ = encode[b & 0xF], --n_out;
	}
	return true;
}

bool HexEncoder::finish(uint8_t *&out, size_t n_out) {
	const uint8_t *in = nullptr;
	return this->process(out, n_out, in, 0);
}


bool HexDecoder::process(uint8_t *&out, size_t n_out, const uint8_t *&in, size_t n_in) {
	static constexpr int8_t decode[55] = {
		0,  1,  2,  3,  4,  5,  6,  7,  8,  9, -1, -1, -1, -1, -1, -1,
		-1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, 10, 11, 12, 13, 14, 15
	};
	uint b0, b1;
	if (buf_full) {
		b0 = buf, buf_full = false;
	}
	else {
		if (n_in == 0) {
			return true;
		}
		b0 = *in++, --n_in;
	}
	while (n_in > 0) {
		if (n_out == 0) {
			return false;
		}
		if ((b0 -= '0') > 'f' - '0' || static_cast<int>(b0 = decode[b0]) < 0 || (--n_in, b1 = *in++ - '0') > 'f' - '0' || static_cast<int>(b1 = decode[b1]) < 0) {
			throw std::ios_base::failure("invalid hex");
		}
		*out++ = static_cast<uint8_t>(b0 << 4 | b1), --n_out;
		if (n_in == 0) {
			return true;
		}
		b0 = *in++, --n_in;
	}
	buf = static_cast<uint8_t>(b0), buf_full = true;
	return true;
}

bool HexDecoder::finish(uint8_t *&, size_t) {
	if (buf_full) {
		throw std::ios_base::failure("invalid hex");
	}
	return true;
}


#include "codec.tcc"

template class CodecSource<HexEncoder>;
template class CodecSource<HexDecoder>;
template class CodecSink<HexEncoder>;
template class CodecSink<HexDecoder>;
