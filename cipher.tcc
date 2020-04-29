#include "cipher.h"

#include <algorithm>
#include <cassert>
#include <stdexcept>

#include "compiler.h"


template <typename Cipher>
constexpr size_t CTR<Cipher>::input_block_size;
template <typename Cipher>
constexpr size_t CTR<Cipher>::output_block_size;

template <typename Cipher>
bool CTR<Cipher>::process(std::byte * _restrict &out, size_t n_out, const std::byte *&in, size_t n_in) {
	while (n_in > 0) {
		if (n_out == 0) {
			return false;
		}
		if (pos == 0) {
			cipher.process(mask, reinterpret_cast<const std::byte *>(iv), sizeof iv);
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
bool CTR<Cipher>::finish(std::byte *&, size_t) {
	if (_unlikely(pos != 0)) {
		throw std::logic_error("incomplete block");
	}
	return true;
}
