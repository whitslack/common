#include <cstddef>
#include <cstdint>
#include <string>


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


std::string encode_base64(const void *in, size_t n_in);

template <typename T>
static inline std::string encode_base64(const T &in) {
	return encode_base64(&in, sizeof in);
}

void decode_base64(void *out, size_t n_out, const char in[], size_t n_in);

static inline void decode_base64(void *out, size_t n_out, const std::string &in) {
	decode_base64(out, n_out, in.data(), in.size());
}

template <typename T>
static inline void decode_base64(T &out, const char in[], size_t n_in) {
	decode_base64(&out, sizeof out, in, n_in);
}

template <typename T>
static inline void decode_base64(T &out, const std::string &in) {
	decode_base64(&out, sizeof out, in.data(), in.size());
}
