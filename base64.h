#include <cstddef>
#include <cstdint>


class Base64Encoder {

private:
	uint8_t state, buf;

public:
	Base64Encoder() : state() { }

public:
	bool process(uint8_t *&out, size_t n_out, const uint8_t *&in, size_t n_in);
	bool finish(uint8_t *&out, size_t n_out);

};


class Base64Decoder {

private:
	uint8_t state, buf;

public:
	Base64Decoder() : state() { }

public:
	bool process(uint8_t *&out, size_t n_out, const uint8_t *&in, size_t n_in);
	bool finish(uint8_t *&out, size_t n_out);

};
