#include <cstddef>

#include "compiler.h"


class HexDumpEncoder {

private:
	std::byte buf[16];
	unsigned out_addr = 0, state = 0;

public:
	bool process(std::byte * _restrict &out, size_t n_out, const std::byte *&in, size_t n_in);
	bool finish(std::byte *&out, size_t n_out);

};
