#include "cipher.h"

#include <algorithm>
#include <cassert>

#include "compiler.h"


template <typename Cipher>
constexpr size_t CTR<Cipher>::input_block_size;
template <typename Cipher>
constexpr size_t CTR<Cipher>::output_block_size;

template <typename Cipher>
bool CTR<Cipher>::process(uint8_t *&out, size_t n_out, const uint8_t *&in, size_t n_in) {
	while (n_in > 0) {
		if (n_out == 0) {
			return false;
		}
		if (pos == 0) {
			cipher.process(mask, iv, sizeof iv);
			for (size_t i = input_block_size; i > 0 && ++iv[--i] != 0;);
		}
		*out++ = *in++ ^ mask[pos], --n_in, --n_out;
		if (++pos == sizeof mask) {
			pos = 0;
		}
	}
	return true;
}

template <typename Cipher>
bool CTR<Cipher>::finish(uint8_t *&, size_t) {
	if (pos != 0) {
		throw std::logic_error("incomplete block");
	}
	return true;
}
