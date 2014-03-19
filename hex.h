#include <cstddef>
#include <cstdint>


class HexEncoder {

private:
	bool buf_full;
	uint8_t buf;

public:
	HexEncoder() : buf_full() { }

public:
	bool process(uint8_t *&out, size_t n_out, const uint8_t *&in, size_t n_in);
	bool finish(uint8_t *&out, size_t n_out);

};


class HexDecoder {

private:
	bool buf_full;
	uint8_t buf;

public:
	HexDecoder() : buf_full() { }

public:
	bool process(uint8_t *&out, size_t n_out, const uint8_t *&in, size_t n_in);
	bool finish(uint8_t *&out, size_t n_out);

};
