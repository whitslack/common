#include "hash.h"


class SHA1 : public Hash<64, 20, 20, uint32_t, uint64_t, true> {

public:
	SHA1();

protected:
	void update(const uint8_t (&block)[64]) override;

};


template <size_t Digest_Size>
class SHA256Base : public Hash<64, 32, Digest_Size, uint32_t, uint64_t, true> {

protected:
	SHA256Base(const uint32_t (&init)[8]) : Hash<64, 32, Digest_Size, uint32_t, uint64_t, true>(init) { }

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
#include "bigint.h"
typedef uint128_t sha512_length_t;
#endif

template <size_t Digest_Size>
class SHA512Base : public Hash<128, 64, Digest_Size, uint64_t, sha512_length_t, true> {

protected:
	SHA512Base(const uint64_t (&init)[8]) : Hash<128, 64, Digest_Size, uint64_t, sha512_length_t, true>(init) { }

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
