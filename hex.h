#include <cstddef>
#include <cstdint>

#include "compiler.h"


class HexEncoder {

private:
	bool buf_full = false;
	uint8_t buf = 0;

public:
	bool process(uint8_t * _restrict &out, size_t n_out, const uint8_t *&in, size_t n_in) noexcept;
	bool finish(uint8_t *&out, size_t n_out) noexcept;

};


class HexDecoder {

private:
	bool buf_full = false;
	uint8_t buf = 0;

public:
	bool process(uint8_t * _restrict &out, size_t n_out, const uint8_t *&in, size_t n_in);
	bool finish(uint8_t *&out, size_t n_out);

};
