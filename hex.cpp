#include "hex.h"

#include <ios>


const char HexEncoder::encode_lower[16] = {
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
};

const char HexEncoder::encode_upper[16] = {
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
};

bool HexEncoder::process(std::byte * _restrict &out, size_t n_out, const std::byte *&in, size_t n_in) noexcept {
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
		b = static_cast<unsigned>(*in++), --n_in;
		*out++ = static_cast<std::byte>(xdigit_lower(b >> 4)), --n_out;
buf_full:
		if (_unlikely(n_out == 0)) {
			buf_full = true, buf = static_cast<uint8_t>(b);
			return false;
		}
		*out++ = static_cast<std::byte>(xdigit_lower(b & 0xF)), --n_out;
	}
}

bool HexEncoder::finish(std::byte *&out, size_t n_out) noexcept {
	const std::byte *in = nullptr;
	return this->process(out, n_out, in, 0);
}


constexpr int8_t HexDecoder::decode['f' - '0' + 1] = {
	 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, -1, -1, -1, -1, -1, -1,
	-1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, 10, 11, 12, 13, 14, 15
};

bool HexDecoder::process(std::byte * _restrict &out, size_t n_out, const std::byte *&in, size_t n_in) {
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
		b0 = static_cast<unsigned>(*in++), --n_in;
		if (_unlikely(static_cast<int>(b0 = xdigit_value(static_cast<char>(b0))) < 0)) {
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
		b1 = static_cast<unsigned>(*in++), --n_in;
		if (_unlikely(static_cast<int>(b1 = xdigit_value(static_cast<char>(b1))) < 0)) {
			throw std::ios_base::failure("invalid hex");
		}
		*out++ = static_cast<std::byte>(b0 << 4 | b1), --n_out;
	}
}

bool HexDecoder::finish(std::byte *&, size_t) {
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
