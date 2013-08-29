#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <functional>
#include <streambuf>

#include "io.h"


template <size_t Block_Size, size_t State_Size, size_t Digest_Size, typename Word_Type, typename Length_Type>
class SHABase : public Sink {

public:
	typedef Word_Type word_type;
	typedef Length_Type length_type;

public:
	static constexpr size_t block_size = Block_Size, digest_size = Digest_Size;

protected:
	static constexpr size_t state_size = State_Size;

protected:
	word_type state[state_size / sizeof(word_type)];

private:
	length_type length;
	uint8_t buffer[block_size];
	size_t buffer_pos;

protected:
	SHABase(const word_type (&init)[state_size / sizeof(word_type)]) : length(), buffer_pos() {
		std::memcpy(state, init, state_size);
	}

public:
	size_t write(const void *buf, size_t n, bool more = false) override;
	const uint8_t (& digest())[digest_size];

protected:
	virtual void update(const uint8_t (&block)[block_size]) = 0;

};


class SHA1 : public SHABase<64, 20, 20, uint32_t, uint64_t> {

public:
	SHA1();

protected:
	void update(const uint8_t (&block)[64]) override;

};


template <size_t Digest_Size>
class SHA256Base : public SHABase<64, 32, Digest_Size, uint32_t, uint64_t> {

protected:
	SHA256Base(const uint32_t (&init)[8]) : SHABase<64, 32, Digest_Size, uint32_t, uint64_t>(init) { }

protected:
	void update(const uint8_t (&block)[64]) override;

};


class SHA256 : public SHA256Base<32> {

public:
	SHA256();

};


class SHA224 : public SHA256Base<28> {

public:
	SHA224();

};


#ifdef __SIZEOF_INT128__
typedef unsigned __int128 sha512_length_t;
#else
struct sha512_length_t {
	uint64_t low, high;
};
#endif

template <size_t Digest_Size>
class SHA512Base : public SHABase<128, 64, Digest_Size, uint64_t, sha512_length_t> {

protected:
	SHA512Base(const uint64_t (&init)[8]) : SHABase<128, 64, Digest_Size, uint64_t, sha512_length_t>(init) { }

protected:
	void update(const uint8_t (&block)[128]) override;

};


class SHA512 : public SHA512Base<64> {

public:
	SHA512();

};


class SHA384 : public SHA512Base<48> {

public:
	SHA384();

};
