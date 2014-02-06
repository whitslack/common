#include <cstddef>
#include <cstdint>


class HexEncoder {

private:
	uint8_t buf;
	bool buf_full;

public:
	HexEncoder() : buf_full() { }

public:
	bool process(uint8_t *&out, size_t n_out, const uint8_t *&in, size_t n_in);
	bool finish(uint8_t *&out, size_t n_out);

};


class HexDecoder {

private:
	uint8_t buf;
	bool buf_full;

public:
	HexDecoder() : buf_full() { }

public:
	bool process(uint8_t *&out, size_t n_out, const uint8_t *&in, size_t n_in);
	bool finish(uint8_t *&out, size_t n_out);

};
