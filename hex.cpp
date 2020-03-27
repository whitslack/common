#include "hex.h"

#include <ios>


bool HexEncoder::process(uint8_t * _restrict &out, size_t n_out, const uint8_t *&in, size_t n_in) noexcept {
	static constexpr char encode[16] = {
		'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
	};
	unsigned b;
	if (buf_full) {
		b = buf;
		goto buf_full;
	}
	for (;;) {
		if (_unlikely(n_in == 0)) {
			buf_full = false;
			return true;
		}
		if (_unlikely(n_out == 0)) {
			buf_full = false;
			return false;
		}
		b = *in++, --n_in;
		*out++ = encode[b >> 4], --n_out;
buf_full:
		if (_unlikely(n_out == 0)) {
			buf_full = true, buf = static_cast<uint8_t>(b);
			return false;
		}
		*out++ = encode[b & 0xF], --n_out;
	}
}

bool HexEncoder::finish(uint8_t *&out, size_t n_out) noexcept {
	const uint8_t *in = nullptr;
	return this->process(out, n_out, in, 0);
}


bool HexDecoder::process(uint8_t * _restrict &out, size_t n_out, const uint8_t *&in, size_t n_in) {
	static constexpr int8_t decode[55] = {
		0,  1,  2,  3,  4,  5,  6,  7,  8,  9, -1, -1, -1, -1, -1, -1,
		-1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, 10, 11, 12, 13, 14, 15
	};
	unsigned b0, b1;
	if (buf_full) {
		b0 = buf;
		goto buf_full;
	}
	for (;;) {
		if (_unlikely(n_in == 0)) {
			buf_full = false;
			return true;
		}
		b0 = *in++, --n_in;
		if (_unlikely((b0 -= '0') > 'f' - '0' || static_cast<int>(b0 = decode[b0]) < 0)) {
			throw std::ios_base::failure("invalid hex");
		}
buf_full:
		if (_unlikely(n_in == 0)) {
			buf_full = true, buf = static_cast<uint8_t>(b0);
			return true;
		}
		if (_unlikely(n_out == 0)) {
			buf_full = true, buf = static_cast<uint8_t>(b0);
			return false;
		}
		b1 = *in++, --n_in;
		if (_unlikely((b1 -= '0') > 'f' - '0' || static_cast<int>(b1 = decode[b1]) < 0)) {
			throw std::ios_base::failure("invalid hex");
		}
		*out++ = static_cast<uint8_t>(b0 << 4 | b1), --n_out;
	}
}

bool HexDecoder::finish(uint8_t *&, size_t) {
	if (_unlikely(buf_full)) {
		throw std::ios_base::failure("invalid hex");
	}
	return true;
}


#include "codec.tcc"

template class CodecSource<HexEncoder>;
template class CodecSource<HexDecoder>;
template class CodecSink<HexEncoder>;
template class CodecSink<HexDecoder>;
