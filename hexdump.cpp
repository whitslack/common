#include "hexdump.h"

#include <cctype>
#include <stdexcept>


bool HexDumpEncoder::process(uint8_t * _restrict &out, size_t n_out, const uint8_t *&in, size_t n_in) {
	static constexpr char encode[16] = {
		'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
	};
	for (;;) {
		switch (state) {
			case 0:
				if (n_in == 0) {
					return true;
				}
				// fall through
			case 1: case 2: case 3: case 4: case 5: case 6: case 7:
				if (n_out == 0) {
					return false;
				}
				*out++ = encode[out_addr >> (7 - state) * 4 & 0xF], --n_out, ++state;
				break;
			case 8: case 9:
			case 12: case 15: case 18: case 21: case 24: case 27: case 30: case 33: case 34:
			case 37: case 40: case 43: case 46: case 49: case 52: case 55: case 58: case 59:
				if (n_out == 0) {
					return false;
				}
				*out++ = ' ', --n_out, ++state;
				break;
			case 10: case 13: case 16: case 19: case 22: case 25: case 28: case 31:
			case 35: case 38: case 41: case 44: case 47: case 50: case 53: case 56: {
				if (n_in == 0) {
					return true;
				}
				if (n_out == 0) {
					return false;
				}
				auto pos = (state - 10) / 3;
				int b = buf[pos] = *in++; --n_in;
				*out++ = encode[b >> 4], --n_out, ++state;
			}
			case 11: case 14: case 17: case 20: case 23: case 26: case 29: case 32:
			case 36: case 39: case 42: case 45: case 48: case 51: case 54: case 57: {
				if (n_out == 0) {
					return false;
				}
				auto pos = (state - 10) / 3;
				int b = buf[pos];
				*out++ = encode[b & 0xF], --n_out, ++state;
				break;
			}
			case 60: case 77:
				if (n_out == 0) {
					return false;
				}
				*out++ = '|', --n_out, ++state;
				break;
			case 61: case 62: case 63: case 64: case 65: case 66: case 67: case 68:
			case 69: case 70: case 71: case 72: case 73: case 74: case 75: case 76: {
				if (n_out == 0) {
					return false;
				}
				int b = buf[state - 61];
				*out++ = static_cast<uint8_t>(std::isprint(b) ? b : '.'), --n_out, ++state;
				break;
			}
			case 78:
				if (n_out == 0) {
					return false;
				}
				*out++ = '\n', --n_out, state = 0, out_addr += 16;
				break;
			default:
				throw std::logic_error("illegal state");
		}
	}
}

bool HexDumpEncoder::finish(uint8_t *&out, size_t n_out) {
	unsigned pos;
	if (state >> 8 == 0) {
		const uint8_t *in = nullptr;
		if (!this->process(out, n_out, in, 0)) {
			return false;
		}
		state |= (pos = (state - 10) / 3) << 8;
	}
	else {
		pos = state >> 8;
	}
	unsigned col = state & 0xFF;
	if (col == 0) {
		return true;
	}
	while (n_out > 0) {
		if (col < 60) {
			*out++ = ' ', --n_out, ++state, ++col;
		}
		else if (col == 60 || col == 61 + pos) {
			*out++ = '|', --n_out, ++state, ++col;
		}
		else if (col == 62 + pos) {
			*out++ = '\n', --n_out, state &= ~0xFF;
			return true;
		}
		else {
			int b = buf[col - 61];
			*out++ = static_cast<uint8_t>(std::isprint(b) ? b : '.'), --n_out, ++state;
			break;
		}
	}
	return false;
}


#include "codec.tcc"

template class CodecSource<HexDumpEncoder>;
template class CodecSink<HexDumpEncoder>;
