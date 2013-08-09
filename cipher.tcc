#include "cipher.h"

#include <algorithm>
#include <cassert>

#include "compiler.h"


template <typename Cipher>
constexpr size_t CTR<Cipher>::input_block_size;
template <typename Cipher>
constexpr size_t CTR<Cipher>::output_block_size;

template <typename Cipher>
size_t CTR<Cipher>::process(uint8_t (&out)[CTR<Cipher>::output_block_size], const uint8_t in[], size_t n) {
	assert(n <= input_block_size);
	size_t r _unused = cipher.process(out, iv, sizeof iv);
	assert(r == output_block_size);
	for (size_t i = 0; i < n; ++i) {
		out[i] ^= in[i];
	}
	for (size_t i = input_block_size; i > 0 && ++iv[--i] != 0;);
	return n;
}
