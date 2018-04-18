#include <cstddef>
#include <cstdint>

#include "compiler.h"


class Base64Encoder {

private:
	uint8_t state = 0, buf = 0;

public:
	bool process(uint8_t * _restrict &out, size_t n_out, const uint8_t *&in, size_t n_in);
	bool finish(uint8_t *&out, size_t n_out);

};


class Base64Decoder {

private:
	uint8_t state = 0, buf = 0;

public:
	bool process(uint8_t * _restrict &out, size_t n_out, const uint8_t *&in, size_t n_in);
	bool finish(uint8_t *&out, size_t n_out);

};
