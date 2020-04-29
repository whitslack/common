#include <cstddef>
#include <cstring>
#include <utility>

#include "compiler.h"


template <typename Cipher>
class CTR {
	static_assert(Cipher::input_block_size == Cipher::output_block_size, "");

public:
	typedef Cipher cipher_type;

public:
	static constexpr size_t input_block_size = Cipher::input_block_size;
	static constexpr size_t output_block_size = Cipher::output_block_size;

private:
	uint8_t iv[input_block_size];
	std::byte mask[output_block_size];
	size_t pos;
	cipher_type cipher;

public:
	template <typename... Args>
	explicit CTR(const std::byte (&iv)[input_block_size], Args&&... args) : pos(), cipher(std::forward<Args>(args)...) {
		std::memcpy(this->iv, iv, sizeof iv);
	}

public:
	bool process(std::byte * _restrict &out, size_t n_out, const std::byte *&in, size_t n_in);
	bool finish(std::byte *&out, size_t n_out);

};
