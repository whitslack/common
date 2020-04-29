#include <cstddef>
#include <cstdint>

#include "compiler.h"


class HexEncoder {

private:
	bool buf_full = false;
	uint8_t buf = 0;

public:
	bool process(std::byte * _restrict &out, size_t n_out, const std::byte *&in, size_t n_in) noexcept;
	bool finish(std::byte *&out, size_t n_out) noexcept;

};


class HexDecoder {

private:
	bool buf_full = false;
	uint8_t buf = 0;

public:
	bool process(std::byte * _restrict &out, size_t n_out, const std::byte *&in, size_t n_in);
	bool finish(std::byte *&out, size_t n_out);

};
