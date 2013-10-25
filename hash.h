#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>

#include "io.h"


template <size_t Block_Size, size_t State_Size, size_t Digest_Size, typename Word_Type, typename Length_Type, bool Big_Endian>
class Hash : public Sink {

public:
	typedef Word_Type word_type;
	typedef Length_Type length_type;

public:
	static constexpr size_t block_size = Block_Size, digest_size = Digest_Size;
	static constexpr bool big_endian = Big_Endian;

protected:
	static constexpr size_t state_size = State_Size;

protected:
	word_type state[state_size / sizeof(word_type)];

private:
	length_type length;
	uint8_t buffer[block_size];
	size_t buffer_pos;

protected:
	Hash(const word_type (&init)[state_size / sizeof(word_type)]) : length(), buffer_pos() {
		std::memcpy(state, init, state_size);
	}

public:
	size_t write(const void *buf, size_t n, bool more = false) override;
	const uint8_t (& digest())[digest_size];

protected:
	virtual void update(const uint8_t (&block)[block_size]) = 0;

};