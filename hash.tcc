#include "hash.h"

#include "endian.h"


template <size_t Block_Size, size_t State_Size, size_t Digest_Size, typename Word_Type, typename Length_Type, bool Big_Endian>
constexpr size_t Hash<Block_Size, State_Size, Digest_Size, Word_Type, Length_Type, Big_Endian>::block_size;
template <size_t Block_Size, size_t State_Size, size_t Digest_Size, typename Word_Type, typename Length_Type, bool Big_Endian>
constexpr size_t Hash<Block_Size, State_Size, Digest_Size, Word_Type, Length_Type, Big_Endian>::digest_size;
template <size_t Block_Size, size_t State_Size, size_t Digest_Size, typename Word_Type, typename Length_Type, bool Big_Endian>
constexpr size_t Hash<Block_Size, State_Size, Digest_Size, Word_Type, Length_Type, Big_Endian>::state_size;
template <size_t Block_Size, size_t State_Size, size_t Digest_Size, typename Word_Type, typename Length_Type, bool Big_Endian>
constexpr bool Hash<Block_Size, State_Size, Digest_Size, Word_Type, Length_Type, Big_Endian>::big_endian;

template <size_t Block_Size, size_t State_Size, size_t Digest_Size, typename Word_Type, typename Length_Type, bool Big_Endian>
size_t Hash<Block_Size, State_Size, Digest_Size, Word_Type, Length_Type, Big_Endian>::write(const void *buf, size_t n) {
	size_t ret = n;
	const void *block = buf;
	if (buffer_pos > 0) {
		size_t r = block_size - buffer_pos;
		if (n < r) {
			std::memcpy(buffer + buffer_pos, buf, n);
			buffer_pos += n;
			return n;
		}
		std::memcpy(buffer + buffer_pos, buf, r);
		buf = static_cast<const char *>(buf) - buffer_pos, n += buffer_pos;
		buffer_pos = 0;
		block = buffer;
	}
	while (n >= block_size) {
		this->update(*static_cast<const uint8_t (*)[block_size]>(block));
		length += block_size;
		block = buf = static_cast<const char *>(buf) + block_size;
		n -= block_size;
	}
	if (n > 0) {
		std::memcpy(buffer, buf, n);
		buffer_pos = n;
	}
	return ret;
}

template <size_t Block_Size, size_t State_Size, size_t Digest_Size, typename Word_Type, typename Length_Type, bool Big_Endian>
const typename Hash<Block_Size, State_Size, Digest_Size, Word_Type, Length_Type, Big_Endian>::digest_type & Hash<Block_Size, State_Size, Digest_Size, Word_Type, Length_Type, Big_Endian>::digest() {
	std::conditional_t<Big_Endian, be<Length_Type>, le<Length_Type>> length = this->length + buffer_pos << 3;
	uint8_t marker = 0x80;
	this->write(&marker, sizeof marker);
	if (buffer_pos + sizeof length > block_size) {
		std::memset(buffer + buffer_pos, 0, block_size - buffer_pos);
		this->update(buffer);
		buffer_pos = 0;
	}
	std::memset(buffer + buffer_pos, 0, block_size - sizeof length - buffer_pos);
	std::memcpy(buffer + block_size - sizeof length, &length, sizeof length);
	this->update(buffer);
	buffer_pos = 0;
	for (size_t i = 0; i < digest_size / sizeof(word_type); ++i) {
		reinterpret_cast<std::conditional_t<Big_Endian, be<Word_Type>, le<Word_Type>> &>(state[i]) = state[i];
	}
	return *reinterpret_cast<const digest_type *>(state);
}
