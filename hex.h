#include <cassert>
#include <cstddef>
#include <cstdint>

#include "compiler.h"
#include "memory.h"


class HexEncoder {

private:
	static const char encode_lower[16], encode_upper[16];

public:
	static char _const xdigit_lower(unsigned value) noexcept {
		assert((value & ~0xF) == 0);
		return encode_lower[value];
	}
	static char _const xdigit_upper(unsigned value) noexcept {
		assert((value & ~0xF) == 0);
		return encode_upper[value];
	}

private:
	bool buf_full = false;
	uint8_t buf = 0;

public:
	bool process(std::byte * _restrict &out, size_t n_out, const std::byte *&in, size_t n_in) noexcept;
	bool finish(std::byte *&out, size_t n_out) noexcept;

};


class HexDecoder {

private:
	static const int8_t decode['f' - '0' + 1];

public:
	static int _const xdigit_value(char xdigit) noexcept {
		unsigned idx = static_cast<unsigned char>(xdigit) - '0';
		return _likely(idx < std::size(decode)) ? decode[idx] : -1;
	}

private:
	bool buf_full = false;
	uint8_t buf = 0;

public:
	bool process(std::byte * _restrict &out, size_t n_out, const std::byte *&in, size_t n_in);
	bool finish(std::byte *&out, size_t n_out);

};
