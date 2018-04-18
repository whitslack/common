#include <cstddef>
#include <cstdint>

#include "compiler.h"


class HexDumpEncoder {

private:
	uint8_t buf[16];
	unsigned out_addr = 0, state = 0;

public:
	bool process(uint8_t * _restrict &out, size_t n_out, const uint8_t *&in, size_t n_in);
	bool finish(uint8_t *&out, size_t n_out);

};
