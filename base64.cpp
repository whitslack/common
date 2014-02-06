#include "base64.h"

#include <ios>
#include <stdexcept>


static constexpr char encode[64] = {
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
	'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
	'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
	'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'
};

bool Base64Encoder::process(uint8_t *&out, size_t n_out, const uint8_t *&in, size_t n_in) {
	for (;;) {
		switch (buf_state) {
			case 0:
				if (n_in == 0) {
					return true;
				}
				if (n_out == 0) {
					return false;
				}
				*out++ = encode[(buf = *in++) >> 2], --n_in, buf_state = 1, --n_out;
			case 1: {
				if (n_in == 0) {
					return true;
				}
				if (n_out == 0) {
					return false;
				}
				uint8_t b = buf;
				*out++ = encode[b << 4 & (1 << 6) - 1 | (buf = *in++) >> 4], --n_in, buf_state = 2, --n_out;
			}
			case 2: {
				if (n_in == 0) {
					return true;
				}
				if (n_out == 0) {
					return false;
				}
				uint8_t b = buf;
				*out++ = encode[b << 2 & (1 << 6) - 1 | (buf = *in++) >> 6], --n_in, buf_state = 3, --n_out;
			}
			case 3:
				if (n_out == 0) {
					return false;
				}
				*out++ = encode[buf & (1 << 6) - 1], buf_state = 0, --n_out;
				break;
			default:
				throw std::logic_error("illegal state");
		}
	}
}

bool Base64Encoder::finish(uint8_t *&out, size_t n_out) {
	switch (buf_state) {
		case 1:
			if (n_out == 0) {
				return false;
			}
			*out++ = encode[buf << 4 & (1 << 6) - 1], buf_state = 4, --n_out;
		case 4:
			if (n_out == 0) {
				return false;
			}
			*out++ = '=', buf_state = 5, --n_out;
			goto case5;
		case 2:
			if (n_out == 0) {
				return false;
			}
			*out++ = encode[buf << 2 & (1 << 6) - 1], buf_state = 5, --n_out;
		case 5:
		case5:
			if (n_out == 0) {
				return false;
			}
			*out++ = '=', buf_state = 0, --n_out;
			return true;
		case 3:
			if (n_out == 0) {
				return false;
			}
			*out++ = encode[buf & (1 << 6) - 1], buf_state = 0, --n_out;
	}
	return true;
}


bool Base64Decoder::process(uint8_t *&out, size_t n_out, const uint8_t *&in, size_t n_in) {
	static constexpr int8_t decode[80] = {
		62, -1, -1, -1, 63, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1,
		-1, -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17,
		18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1, -1, 26, 27, 28, 29, 30, 31,
		32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51
	};
	for (;;) {
		switch (buf_state) {
			case 0: {
				if (n_in == 0) {
					return true;
				}
				uint b;
				if ((--n_in, b = *in++ - '+') > 'z' - '+' || static_cast<int>(b = decode[b]) < 0) {
					throw std::ios_base::failure("invalid base64");
				}
				buf = static_cast<uint8_t>(b), buf_state = 1;
			}
			case 1: {
				if (n_in == 0) {
					return true;
				}
				if (n_out == 0) {
					return false;
				}
				uint b;
				if ((--n_in, b = *in++ - '+') > 'z' - '+' || static_cast<int>(b = decode[b]) < 0) {
					buf_state = 0;
					throw std::ios_base::failure("invalid base64");
				}
				*out++ = static_cast<uint8_t>(buf << 2 | b >> 4), --n_out;
				buf = static_cast<uint8_t>(b), buf_state = 2;
			}
			case 2: {
				if (n_in == 0) {
					return true;
				}
				if (*in == '=') {
					++in, --n_in, buf_state = 4;
					break;
				}
				if (n_out == 0) {
					return false;
				}
				uint b = *in++; --n_in;
				if ((b -= '+') > 'z' - '+' || static_cast<int>(b = decode[b]) < 0) {
					buf_state = 0;
					throw std::ios_base::failure("invalid base64");
				}
				*out++ = static_cast<uint8_t>(buf << 4 | b >> 2), --n_out;
				buf = static_cast<uint8_t>(b), buf_state = 3;
			}
			case 3: {
				if (n_in == 0) {
					return true;
				}
				if (*in == '=') {
					++in, --n_in, buf_state = 5;
					break;
				}
				if (n_out == 0) {
					return false;
				}
				uint b = *in++; --n_in;
				if ((b -= '+') > 'z' - '+' || static_cast<int>(b = decode[b]) < 0) {
					buf_state = 0;
					throw std::ios_base::failure("invalid base64");
				}
				*out++ = static_cast<uint8_t>(buf << 6 | b), --n_out;
				buf_state = 0;
				break;
			}
			case 4:
				if (n_in == 0) {
					return true;
				}
				if (--n_in, *in++ != '=') {
					buf_state = 0;
					throw std::ios_base::failure("invalid base64");
				}
				buf_state = 5;
			case 5:
				if (n_in == 0) {
					return true;
				}
				buf_state = 0;
				throw std::ios_base::failure("invalid base64");
		}
	}
}

bool Base64Decoder::finish(uint8_t *&, size_t) {
	if (buf_state != 0 && buf_state != 5) {
		throw std::ios_base::failure("invalid base64");
	}
	return true;
}


#include "codec.tcc"

template class CodecSource<Base64Encoder>;
template class CodecSource<Base64Decoder>;
template class CodecSink<Base64Encoder>;
template class CodecSink<Base64Decoder>;
