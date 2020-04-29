#include <cstddef>
#include <cstdint>

#include "compiler.h"


class Base64Encoder {

private:
	uint8_t state = 0, buf = 0;

public:
	bool process(std::byte * _restrict &out, size_t n_out, const std::byte *&in, size_t n_in);
	bool finish(std::byte *&out, size_t n_out);

};


class Base64Decoder {

private:
	uint8_t state = 0, buf = 0;

public:
	bool process(std::byte * _restrict &out, size_t n_out, const std::byte *&in, size_t n_in);
	bool finish(std::byte *&out, size_t n_out);

};
