#include <cstddef>
#include <cstdint>


class Base64Encoder {

public:
	static constexpr size_t input_block_size = 3, output_block_size = 4;

public:
	size_t process(uint8_t (&out)[4], const uint8_t in[], size_t n);

};


class Base64Decoder {

public:
	static constexpr size_t input_block_size = 4, output_block_size = 3;

public:
	size_t process(uint8_t (&out)[3], const uint8_t in[], size_t n);

};
