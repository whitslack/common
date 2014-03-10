#include <cstddef>
#include <cstring>
#include <utility>


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
	uint8_t mask[output_block_size];
	size_t pos;
	cipher_type cipher;

public:
	template <typename... Args>
	explicit CTR(const uint8_t (&iv)[input_block_size], Args&&... args) : pos(), cipher(std::forward<Args>(args)...) {
		std::memcpy(this->iv, iv, sizeof iv);
	}

public:
	bool process(uint8_t *&out, size_t n_out, const uint8_t *&in, size_t n_in);
	bool finish(uint8_t *&out, size_t n_out);

};
