#include <cstddef>
#include <cstdint>


class HexEncoder {

public:
	static constexpr size_t input_block_size = 1, output_block_size = 2;

public:
	size_t process(uint8_t (&out)[2], const uint8_t in[], size_t n);

};


class HexDecoder {

public:
	static constexpr size_t input_block_size = 2, output_block_size = 1;

public:
	size_t process(uint8_t (&out)[1], const uint8_t in[], size_t n);

};
