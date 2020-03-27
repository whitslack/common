#include "base64.h"

#include <ios>
#include <stdexcept>


static constexpr char encode[64] = {
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
	'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
	'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
	'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'
};

bool Base64Encoder::process(uint8_t * _restrict &out, size_t n_out, const uint8_t *&in, size_t n_in) {
	unsigned b0, b1;
	switch (state) {
		case 0:
			goto state0;
		case 1:
			b0 = buf;
			goto state1;
		case 2:
			b1 = buf;
			goto state2;
		case 3:
			b0 = buf;
			goto state3;
		default:
			throw std::logic_error("illegal state");
	}
	for (;;) {
state0:
		if (n_in == 0) {
			state = 0;
			return true;
		}
		if (n_out == 0) {
			state = 0;
			return false;
		}
		b0 = *in++, --n_in;
		*out++ = encode[b0 >> 2], --n_out;
state1:
		if (n_in == 0) {
			state = 1, buf = static_cast<uint8_t>(b0);
			return true;
		}
		if (n_out == 0) {
			state = 1, buf = static_cast<uint8_t>(b0);
			return false;
		}
		b1 = *in++, --n_in;
		*out++ = encode[b0 << 4 & 0x3F | b1 >> 4], --n_out;
state2:
		if (n_in == 0) {
			state = 2, buf = static_cast<uint8_t>(b1);
			return true;
		}
		if (n_out == 0) {
			state = 2, buf = static_cast<uint8_t>(b1);
			return false;
		}
		b0 = *in++, --n_in;
		*out++ = encode[b1 << 2 & 0x3F | b0 >> 6], --n_out;
state3:
		if (n_out == 0) {
			state = 3, buf = static_cast<uint8_t>(b0);
			return false;
		}
		*out++ = encode[b0 & 0x3F], --n_out;
	}
}

bool Base64Encoder::finish(uint8_t *&out, size_t n_out) {
	switch (state) {
		case 0:
			state = 6;
			return true;
		case 1:
			if (n_out == 0) {
				return false;
			}
			*out++ = encode[buf << 4 & 0x3F], --n_out;
			_fallthrough;
		case 4:
			if (n_out == 0) {
				state = 4;
				return false;
			}
			*out++ = '=', --n_out;
			goto state5;
		case 2:
			if (n_out == 0) {
				return false;
			}
			*out++ = encode[buf << 2 & 0x3F], --n_out;
			_fallthrough;
		case 5:
state5:
			if (n_out == 0) {
				state = 5;
				return false;
			}
			*out++ = '=', --n_out;
			state = 6;
			return true;
		case 3:
			if (n_out == 0) {
				return false;
			}
			*out++ = encode[buf & 0x3F], --n_out;
			state = 6;
			_fallthrough;
		case 6:
			return true;
		default:
			throw std::logic_error("illegal state");
	}
}


bool Base64Decoder::process(uint8_t * _restrict &out, size_t n_out, const uint8_t *&in, size_t n_in) {
	static constexpr int8_t decode[80] = {
		62, -1, -1, -1, 63, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1,
		-1, -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17,
		18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1, -1, 26, 27, 28, 29, 30, 31,
		32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51
	};
	unsigned b0, b1;
	switch (state) {
		case 0:
			goto state0;
		case 1:
			b0 = buf;
			goto state1;
		case 2:
			b1 = buf;
			goto state2;
		case 3:
			b0 = buf;
			goto state3;
		case 4:
			goto state4;
		case 5:
			goto state5;
	}
	for (;;) {
state0:
		if (n_in == 0) {
			state = 0;
			return true;
		}
		b0 = *in++, --n_in;
		if (_unlikely((b0 -= '+') > 'z' - '+' || static_cast<int>(b0 = decode[b0]) < 0)) {
			throw std::ios_base::failure("invalid base64");
		}
state1:
		if (n_in == 0) {
			state = 1, buf = static_cast<uint8_t>(b0);
			return true;
		}
		if (n_out == 0) {
			state = 1, buf = static_cast<uint8_t>(b0);
			return false;
		}
		b1 = *in++, --n_in;
		if (_unlikely((b1 -= '+') > 'z' - '+' || static_cast<int>(b1 = decode[b1]) < 0)) {
			throw std::ios_base::failure("invalid base64");
		}
		*out++ = static_cast<uint8_t>(b0 << 2 | b1 >> 4), --n_out;
state2:
		if (n_in == 0) {
			state = 2, buf = static_cast<uint8_t>(b1);
			return true;
		}
		if (*in == '=') {
			++in, --n_in;
			goto state4;
		}
		if (n_out == 0) {
			state = 2, buf = static_cast<uint8_t>(b1);
			return false;
		}
		b0 = *in++, --n_in;
		if (_unlikely((b0 -= '+') > 'z' - '+' || static_cast<int>(b0 = decode[b0]) < 0)) {
			throw std::ios_base::failure("invalid base64");
		}
		*out++ = static_cast<uint8_t>(b1 << 4 | b0 >> 2), --n_out;
state3:
		if (n_in == 0) {
			state = 3, buf = static_cast<uint8_t>(b0);
			return true;
		}
		if (*in == '=') {
			++in, --n_in;
			goto state5;
		}
		if (n_out == 0) {
			state = 3, buf = static_cast<uint8_t>(b0);
			return false;
		}
		b1 = *in++, --n_in;
		if (_unlikely((b1 -= '+') > 'z' - '+' || static_cast<int>(b1 = decode[b1]) < 0)) {
			throw std::ios_base::failure("invalid base64");
		}
		*out++ = static_cast<uint8_t>(b0 << 6 | b1), --n_out;
	}
state4:
	if (n_in == 0) {
		state = 4;
		return true;
	}
	if (_unlikely(*in != '=')) {
		throw std::ios_base::failure("invalid base64");
	}
	++in, --n_in;
state5:
	if (_likely(n_in == 0)) {
		state = 5;
		return true;
	}
	throw std::ios_base::failure("invalid base64");
}

bool Base64Decoder::finish(uint8_t *&, size_t) {
	if (_unlikely(state != 0 && state != 5)) {
		throw std::ios_base::failure("invalid base64");
	}
	return true;
}


#include "codec.tcc"

template class CodecSource<Base64Encoder>;
template class CodecSource<Base64Decoder>;
template class CodecSink<Base64Encoder>;
template class CodecSink<Base64Decoder>;
