#include "serial.h"


static size_t varint_size(uint8_t first_byte) {
	unsigned i = static_cast<uint8_t>(~first_byte);
	if (i == 0) {
		return 9;
	}
	unsigned n = 2;
	if (i >> 4 == 0) {
		n += 4;
		i <<= 4;
	}
	if (i >> 6 == 0) {
		n += 2;
		i <<= 2;
	}
	return n - (i >> 7);
}

Source & _weak read_varint(Source &source, int32_t &value) {
	int8_t b;
	source.read_fully(&b, sizeof b);
	switch (varint_size(b)) {
		case 1:
			value = (b & 0x3F) + ~((b & 0x40) - 1);
			break;
		case 2: {
			int32_t x = b << 26;
			uint8_t b1;
			source.read_fully(&b1, sizeof b1);
			value = (x >> 18 | b1) + (x >> 31 | 1) * 0x40;
			break;
		}
		case 3: {
			int32_t x = b << 27;
			be<uint16_t> w;
			source.read_fully(&w, sizeof w);
			value = (x >> 11 | w) + (x >> 31 | 1) * 0x2040;
			break;
		}
		case 4: {
			int32_t x = b << 28;
			be<uint32_t> d = 0;
			source.read_fully(reinterpret_cast<uint8_t *>(&d) + 1, sizeof d - 1);
			value = (x >> 4 | d) + (x >> 31 | 1) * 0x102040;
			break;
		}
		case 5: {
			int32_t x = b << 29;
			if (static_cast<uint32_t>((x >> 29) + 1) <= 1) {
				source.read_fully(&value, sizeof value);
				value = as_be(value);
				if (x < 0 ? (value -= 0x08102040) < 0 : (value += 0x08102040) >= 0) {
					break;
				}
			}
		}
		default:
			throw std::ios_base::failure("value is too large");
	}
	return source;
}

Sink & _weak write_varint(Sink &sink, int32_t value) {
	if (value < 0) {
		if (value > ~(1 << 6)) {
			uint8_t b = static_cast<uint8_t>(~(1 << 7) & value);
			sink.write_fully(&b, sizeof b);
		}
		else if ((value += (1 << 6)) > ~(1 << 13)) {
			be<uint16_t> w = static_cast<uint16_t>(~(1 << 14) & value);
			sink.write_fully(&w, sizeof w);
		}
		else if ((value += (1 << 13)) > ~(1 << 20)) {
			be<uint32_t> d = ~(1 << 21) & value;
			sink.write_fully(reinterpret_cast<uint8_t *>(&d) + 1, sizeof d - 1);
		}
		else if ((value += (1 << 20)) > ~(1 << 27)) {
			be<uint32_t> d = ~(1 << 28) & value;
			sink.write_fully(&d, sizeof d);
		}
		else {
			be<uint32_t> a[2] = { 0xF7, value + (1 << 27) };
			sink.write_fully(reinterpret_cast<uint8_t *>(a) + 3, sizeof a - 3);
		}
	}
	else {
		if (value < (1 << 6)) {
			uint8_t b = static_cast<uint8_t>(value);
			sink.write_fully(&b, sizeof b);
		}
		else if ((value -= (1 << 6)) < (1 << 13)) {
			be<uint16_t> w = static_cast<uint16_t>(0x8000 | value);
			sink.write_fully(&w, sizeof w);
		}
		else if ((value -= (1 << 13)) < (1 << 20)) {
			be<uint32_t> d = 0xC00000 | value;
			sink.write_fully(reinterpret_cast<uint8_t *>(&d) + 1, sizeof d - 1);
		}
		else if ((value -= (1 << 20)) < (1 << 27)) {
			be<uint32_t> d = 0xE0000000 | value;
			sink.write_fully(&d, sizeof d);
		}
		else {
			be<uint32_t> a[2] = { 0xF0, value - (1 << 27) };
			sink.write_fully(reinterpret_cast<uint8_t *>(a) + 3, sizeof a - 3);
		}
	}
	return sink;
}

Source & _weak read_varint(Source &source, uint32_t &value) {
	uint8_t b;
	source.read_fully(&b, sizeof b);
	switch (varint_size(b)) {
		case 1:
			value = b;
			break;
		case 2: {
			uint8_t b1;
			source.read_fully(&b1, sizeof b1);
			value = ((b & 0x3F) << 8 | b1) + 0x80;
			break;
		}
		case 3: {
			be<uint16_t> w;
			source.read_fully(&w, sizeof w);
			value = ((b & 0x1F) << 16 | w) + 0x4080;
			break;
		}
		case 4: {
			be<uint32_t> d = 0;
			source.read_fully(reinterpret_cast<uint8_t *>(&d) + 1, sizeof d - 1);
			value = ((b & 0x0F) << 24 | d) + 0x204080;
			break;
		}
		case 5: {
			if ((b & 0x07) == 0) {
				source.read_fully(&value, sizeof value);
				value = as_be(value);
				if (value <= UINT32_MAX - 0x10204080) {
					value += 0x10204080;
					break;
				}
			}
		}
		default:
			throw std::ios_base::failure("value is too large");
	}
	return source;
}

Sink & _weak write_varint(Sink &sink, uint32_t value) {
	if (value < (1 << 7)) {
		uint8_t b = static_cast<uint8_t>(value);
		sink.write_fully(&b, sizeof b);
	}
	else if ((value -= (1 << 7)) < (1 << 14)) {
		be<uint16_t> w = static_cast<uint16_t>(0x8000 | value);
		sink.write_fully(&w, sizeof w);
	}
	else if ((value -= (1 << 14)) < (1 << 21)) {
		be<uint32_t> d = 0xC00000 | value;
		sink.write_fully(reinterpret_cast<uint8_t *>(&d) + 1, sizeof d - 1);
	}
	else if ((value -= (1 << 21)) < (1 << 28)) {
		be<uint32_t> d = 0xE0000000 | value;
		sink.write_fully(&d, sizeof d);
	}
	else {
		be<uint32_t> a[2] = { 0xF0, value - (1 << 28) };
		sink.write_fully(reinterpret_cast<uint8_t *>(a) + 3, sizeof a - 3);
	}
	return sink;
}

Source & _weak read_varint(Source &source, int64_t &value) {
	int8_t b;
	source.read_fully(&b, sizeof b);
	switch (varint_size(b)) {
		case 1:
			value = (b & 0x3F) + ~((b & 0x40) - 1);
			break;
		case 2: {
			int32_t x = b << 26;
			uint8_t b1;
			source.read_fully(&b1, sizeof b1);
			value = (x >> 18 | b1) + (x >> 31 | 1) * 0x40;
			break;
		}
		case 3: {
			int32_t x = b << 27;
			be<uint16_t> w;
			source.read_fully(&w, sizeof w);
			value = (x >> 11 | w) + (x >> 31 | 1) * 0x2040;
			break;
		}
		case 4: {
			int32_t x = b << 28;
			be<uint32_t> d = 0;
			source.read_fully(reinterpret_cast<uint8_t *>(&d) + 1, sizeof d - 1);
			value = static_cast<int32_t>(x >> 4 | d) + (x >> 31 | 1) * 0x102040;
			break;
		}
		case 5: {
			int64_t x = static_cast<int64_t>(b) << 61;
			be<uint32_t> d;
			source.read_fully(&d, sizeof d);
			value = (x >> 29 | d) + (x >> 63 | 1) * 0x08102040;
			break;
		}
		case 6: {
			int64_t x = static_cast<int64_t>(b) << 62;
			be<uint64_t> q = 0;
			source.read_fully(reinterpret_cast<uint8_t *>(&q) + 3, sizeof q - 3);
			value = (x >> 22 | q) + (x >> 63 | 1) * INT64_C(0x0408102040);
			break;
		}
		case 7: {
			int64_t x = static_cast<int64_t>(b) << 63;
			be<uint64_t> q = 0;
			source.read_fully(reinterpret_cast<uint8_t *>(&q) + 2, sizeof q - 2);
			value = (x >> 15 | q) + (x >> 63 | 1) * INT64_C(0x020408102040);
			break;
		}
		case 8: {
			int64_t x;
			source.read_fully(reinterpret_cast<uint8_t *>(&x) + 1, sizeof x - 1);
			x = as_be(x) << 8;
			value = (x >> 8) + (x >> 63 | 1) * INT64_C(0x01020408102040);
			break;
		}
		case 9:
			source.read_fully(&value, sizeof value);
			value = as_be(value);
			if (value < 0 ? (value -= INT64_C(0x0081020408102040)) < 0 : (value += INT64_C(0x0081020408102040)) >= 0) {
				break;
			}
		default:
			throw std::ios_base::failure("value is too large");
	}
	return source;
}

Sink & _weak write_varint(Sink &sink, int64_t value) {
	if (value < 0) {
		if (value > ~(1 << 6)) {
			uint8_t b = static_cast<uint8_t>(~(1 << 7) & value);
			sink.write_fully(&b, sizeof b);
		}
		else if ((value += (1 << 6)) > ~(1 << 13)) {
			be<uint16_t> w = static_cast<uint16_t>(~(1 << 14) & value);
			sink.write_fully(&w, sizeof w);
		}
		else if ((value += (1 << 13)) > ~(1 << 20)) {
			be<uint32_t> d = static_cast<uint32_t>(~(1 << 21) & value);
			sink.write_fully(reinterpret_cast<uint8_t *>(&d) + 1, sizeof d - 1);
		}
		else if ((value += (1 << 20)) > ~(1 << 27)) {
			be<uint32_t> d = static_cast<uint32_t>(~(1 << 28) & value);
			sink.write_fully(&d, sizeof d);
		}
		else if ((value += (1 << 27)) > ~(INT64_C(1) << 34)) {
			be<uint64_t> q = ~(INT64_C(1) << 35) & value;
			sink.write_fully(reinterpret_cast<uint8_t *>(&q) + 3, sizeof q - 3);
		}
		else if ((value += (INT64_C(1) << 34)) > ~(INT64_C(1) << 41)) {
			be<uint64_t> q = ~(INT64_C(1) << 42) & value;
			sink.write_fully(reinterpret_cast<uint8_t *>(&q) + 2, sizeof q - 2);
		}
		else if ((value += (INT64_C(1) << 41)) > ~(INT64_C(1) << 48)) {
			be<uint64_t> q = ~(INT64_C(1) << 49) & value;
			sink.write_fully(reinterpret_cast<uint8_t *>(&q) + 1, sizeof q - 1);
		}
		else if ((value += (INT64_C(1) << 48)) > ~(INT64_C(1) << 55)) {
			be<uint64_t> q = ~(INT64_C(1) << 56) & value;
			sink.write_fully(&q, sizeof q);
		}
		else {
			be<uint64_t> a[2] = { 0xFF, value + (INT64_C(1) << 55) };
			sink.write_fully(reinterpret_cast<uint8_t *>(a) + 7, sizeof a - 7);
		}
	}
	else {
		if (value < (1 << 6)) {
			uint8_t b = static_cast<uint8_t>(value);
			sink.write_fully(&b, sizeof b);
		}
		else if ((value -= (1 << 6)) < (1 << 13)) {
			be<uint16_t> w = static_cast<uint16_t>(0x8000 | value);
			sink.write_fully(&w, sizeof w);
		}
		else if ((value -= (1 << 13)) < (1 << 20)) {
			be<uint32_t> d = static_cast<uint32_t>(0xC00000 | value);
			sink.write_fully(reinterpret_cast<uint8_t *>(&d) + 1, sizeof d - 1);
		}
		else if ((value -= (1 << 20)) < (1 << 27)) {
			be<uint32_t> d = static_cast<uint32_t>(0xE0000000 | value);
			sink.write_fully(&d, sizeof d);
		}
		else if ((value -= (1 << 27)) < (INT64_C(1) << 34)) {
			be<uint64_t> q = INT64_C(0xF000000000) | value;
			sink.write_fully(reinterpret_cast<uint8_t *>(&q) + 3, sizeof q - 3);
		}
		else if ((value -= (INT64_C(1) << 34)) < (INT64_C(1) << 41)) {
			be<uint64_t> q = INT64_C(0xF80000000000) | value;
			sink.write_fully(reinterpret_cast<uint8_t *>(&q) + 2, sizeof q - 2);
		}
		else if ((value -= (INT64_C(1) << 41)) < (INT64_C(1) << 48)) {
			be<uint64_t> q = INT64_C(0xFC000000000000) | value;
			sink.write_fully(reinterpret_cast<uint8_t *>(&q) + 1, sizeof q - 1);
		}
		else if ((value -= (INT64_C(1) << 48)) < (INT64_C(1) << 55)) {
			be<uint64_t> q = INT64_C(0xFE00000000000000) | value;
			sink.write_fully(&q, sizeof q);
		}
		else {
			be<uint64_t> a[2] = { 0xFF, value - (INT64_C(1) << 55) };
			sink.write_fully(reinterpret_cast<uint8_t *>(a) + 7, sizeof a - 7);
		}
	}
	return sink;
}

Source & _weak read_varint(Source &source, uint64_t &value) {
	uint8_t b;
	source.read_fully(&b, sizeof b);
	switch (varint_size(b)) {
		case 1:
			value = b;
			break;
		case 2: {
			uint8_t b1;
			source.read_fully(&b1, sizeof b1);
			value = ((b & 0x3F) << 8 | b1) + 0x80;
			break;
		}
		case 3: {
			be<uint16_t> w;
			source.read_fully(&w, sizeof w);
			value = ((b & 0x1F) << 16 | w) + 0x4080;
			break;
		}
		case 4: {
			be<uint32_t> d = 0;
			source.read_fully(reinterpret_cast<uint8_t *>(&d) + 1, sizeof d - 1);
			value = ((b & 0x0F) << 24 | d) + 0x204080;
			break;
		}
		case 5: {
			be<uint32_t> d;
			source.read_fully(&d, sizeof d);
			value = (static_cast<uint64_t>(b & 0x07) << 32 | d) + 0x10204080;
			break;
		}
		case 6: {
			be<uint64_t> q = 0;
			source.read_fully(reinterpret_cast<uint8_t *>(&q) + 3, sizeof q - 3);
			value = (static_cast<uint64_t>(b & 0x03) << 40 | q) + UINT64_C(0x0810204080);
			break;
		}
		case 7: {
			be<uint64_t> q = 0;
			source.read_fully(reinterpret_cast<uint8_t *>(&q) + 2, sizeof q - 2);
			value = (static_cast<uint64_t>(b & 0x01) << 48 | q) + UINT64_C(0x040810204080);
			break;
		}
		case 8: {
			be<uint64_t> q = 0;
			source.read_fully(reinterpret_cast<uint8_t *>(&q) + 1, sizeof q - 1);
			value = q + UINT64_C(0x02040810204080);
			break;
		}
		case 9:
			source.read_fully(&value, sizeof value);
			value = as_be(value);
			if (value <= UINT64_MAX - UINT64_C(0x0102040810204080)) {
				value += UINT64_C(0x0102040810204080);
				break;
			}
		default:
			throw std::ios_base::failure("value is too large");
	}
	return source;
}

Sink & _weak write_varint(Sink &sink, uint64_t value) {
	if (value < (1 << 7)) {
		uint8_t b = static_cast<uint8_t>(value);
		sink.write_fully(&b, sizeof b);
	}
	else if ((value -= (1 << 7)) < (1 << 14)) {
		be<uint16_t> w = static_cast<uint16_t>(0x8000 | value);
		sink.write_fully(&w, sizeof w);
	}
	else if ((value -= (1 << 14)) < (1 << 21)) {
		be<uint32_t> d = 0xC00000 | static_cast<uint32_t>(value);
		sink.write_fully(reinterpret_cast<uint8_t *>(&d) + 1, sizeof d - 1);
	}
	else if ((value -= (1 << 21)) < (1 << 28)) {
		be<uint32_t> d = 0xE0000000 | static_cast<uint32_t>(value);
		sink.write_fully(&d, sizeof d);
	}
	else if ((value -= (1 << 28)) < (UINT64_C(1) << 35)) {
		be<uint64_t> q = UINT64_C(0xF000000000) | value;
		sink.write_fully(reinterpret_cast<uint8_t *>(&q) + 3, sizeof q - 3);
	}
	else if ((value -= (UINT64_C(1) << 35)) < (UINT64_C(1) << 42)) {
		be<uint64_t> q = UINT64_C(0xF80000000000) | value;
		sink.write_fully(reinterpret_cast<uint8_t *>(&q) + 2, sizeof q - 2);
	}
	else if ((value -= (UINT64_C(1) << 42)) < (UINT64_C(1) << 49)) {
		be<uint64_t> q = UINT64_C(0xFC000000000000) | value;
		sink.write_fully(reinterpret_cast<uint8_t *>(&q) + 1, sizeof q - 1);
	}
	else if ((value -= (UINT64_C(1) << 49)) < (UINT64_C(1) << 56)) {
		be<uint64_t> q = UINT64_C(0xFE00000000000000) | value;
		sink.write_fully(&q, sizeof q);
	}
	else {
		be<uint64_t> a[2] = { 0xFF, value - (UINT64_C(1) << 56) };
		sink.write_fully(reinterpret_cast<uint8_t *>(a) + 7, sizeof a - 7);
	}
	return sink;
}
