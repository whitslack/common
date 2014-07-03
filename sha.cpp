#include "sha.h"


#ifndef __SIZEOF_INT128__

static sha512_length_t & operator += (sha512_length_t &augend, uint64_t addend) {
	uint64_t nlow = augend.low + addend;
	if (nlow < augend.low) {
		++augend.high;
	}
	augend.low = nlow;
	return augend;
}

static sha512_length_t operator + (const sha512_length_t &augend, uint64_t addend) {
	sha512_length_t sum = augend;
	sum += addend;
	return sum;
}

static sha512_length_t & operator <<= (sha512_length_t &operand, size_t shift) {
	while (shift > 0) {
		operand.high <<= 1;
		if (static_cast<int64_t>(operand.low) < 0) {
			operand.high |= 1;
		}
		operand.low <<= 1;
		--shift;
	}
	return operand;
}

static inline sha512_length_t operator << (sha512_length_t operand, size_t shift) {
	return operand <<= shift;
}

static sha512_length_t bswap(const sha512_length_t &v) {
	return sha512_length_t{ bswap(v.high), bswap(v.low) };
}

#if BYTE_ORDER == BIG_ENDIAN
static inline sha512_length_t htobe(const sha512_length_t &v) { return v; }
static inline sha512_length_t betoh(const sha512_length_t &v) { return v; }
static inline sha512_length_t htole(const sha512_length_t &v) { return bswap(v); }
static inline sha512_length_t letoh(const sha512_length_t &v) { return bswap(v); }
#elif BYTE_ORDER == LITTLE_ENDIAN
static inline sha512_length_t htobe(const sha512_length_t &v) { return bswap(v); }
static inline sha512_length_t betoh(const sha512_length_t &v) { return bswap(v); }
static inline sha512_length_t htole(const sha512_length_t &v) { return v; }
static inline sha512_length_t letoh(const sha512_length_t &v) { return v; }
#endif
#endif


#include "hash.tcc"


static const uint32_t sha1_init[5] = { 0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476, 0xc3d2e1f0 };

SHA1::SHA1() : Hash(sha1_init) {
}

void SHA1::update(const uint8_t (&block)[64]) {
	uint32_t words[80];
	for (size_t i = 0; i < 16; ++i) {
		words[i] = be32toh(reinterpret_cast<const uint32_t *>(block)[i]);
	}
	for (size_t i = 16; i < 80; ++i) {
		words[i] = rotl(words[i - 3] ^ words[i - 8] ^ words[i - 14] ^ words[i - 16], 1);
	}
	uint32_t a = state[0], b = state[1], c = state[2], d = state[3], e = state[4];
	for (size_t i = 0; i < 20; ++i) {
		uint32_t temp = e + ((d ^ c) & b ^ d) + rotl(a, 5) + 0x5a827999 + words[i];
		e = d, d = c, c = rotl(b, 30), b = a, a = temp;
	}
	for (size_t i = 20; i < 40; ++i) {
		uint32_t temp = e + (d ^ c ^ b) + rotl(a, 5) + 0x6ed9eba1 + words[i];
		e = d, d = c, c = rotl(b, 30), b = a, a = temp;
	}
	for (size_t i = 40; i < 60; ++i) {
		uint32_t temp = e + (d & (c ^ b) ^ c & b) + rotl(a, 5) + 0x8f1bbcdc + words[i];
		e = d, d = c, c = rotl(b, 30), b = a, a = temp;
	}
	for (size_t i = 60; i < 80; ++i) {
		uint32_t temp = e + (d ^ c ^ b) + rotl(a, 5) + 0xca62c1d6 + words[i];
		e = d, d = c, c = rotl(b, 30), b = a, a = temp;
	}
	state[0] += a, state[1] += b, state[2] += c, state[3] += d, state[4] += e;
}


static const uint32_t sha256_round_constants[64] = {
	0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
	0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
	0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
	0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
	0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
	0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
	0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
	0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

template <size_t Digest_Size>
void SHA256Base<Digest_Size>::update(const uint8_t (&block)[64]) {
	uint32_t words[64];
	for (size_t i = 0; i < 16; ++i) {
		words[i] = be32toh(reinterpret_cast<const uint32_t *>(block)[i]);
	}
	for (size_t i = 16; i < 64; ++i) {
		words[i] = words[i - 16] + (rotr(words[i - 15], 7) ^ rotr(words[i - 15], 18) ^ words[i - 15] >> 3) + words[i - 7] + (rotr(words[i - 2], 17) ^ rotr(words[i - 2], 19) ^ words[i - 2] >> 10);
	}
	uint32_t a = this->state[0], b = this->state[1], c = this->state[2], d = this->state[3], e = this->state[4], f = this->state[5], g = this->state[6], h = this->state[7];
	for (size_t i = 0; i < 64; ++i) {
		uint32_t t1 = h + ((g ^ f) & e ^ g) + (rotr(e, 6) ^ rotr(e, 11) ^ rotr(e, 25)) + sha256_round_constants[i] + words[i];
		h = g, g = f, f = e, e = d + t1;
		uint32_t t2 = (c & (b ^ a) ^ b & a) + (rotr(a, 2) ^ rotr(a, 13) ^ rotr(a, 22));
		d = c, c = b, b = a, a = t1 + t2;
	}
	this->state[0] += a, this->state[1] += b, this->state[2] += c, this->state[3] += d, this->state[4] += e, this->state[5] += f, this->state[6] += g, this->state[7] += h;
}


static const uint32_t sha256_init[8] = {
	0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
};

SHA256::SHA256() : SHA256Base(sha256_init) {
}


static const uint32_t sha224_init[8] = {
	0xc1059ed8, 0x367cd507, 0x3070dd17, 0xf70e5939, 0xffc00b31, 0x68581511, 0x64f98fa7, 0xbefa4fa4
};

SHA224::SHA224() : SHA256Base(sha224_init) {
}


static const uint64_t sha512_round_constants[80] = {
	UINT64_C(0x428a2f98d728ae22), UINT64_C(0x7137449123ef65cd), UINT64_C(0xb5c0fbcfec4d3b2f), UINT64_C(0xe9b5dba58189dbbc),
	UINT64_C(0x3956c25bf348b538), UINT64_C(0x59f111f1b605d019), UINT64_C(0x923f82a4af194f9b), UINT64_C(0xab1c5ed5da6d8118),
	UINT64_C(0xd807aa98a3030242), UINT64_C(0x12835b0145706fbe), UINT64_C(0x243185be4ee4b28c), UINT64_C(0x550c7dc3d5ffb4e2),
	UINT64_C(0x72be5d74f27b896f), UINT64_C(0x80deb1fe3b1696b1), UINT64_C(0x9bdc06a725c71235), UINT64_C(0xc19bf174cf692694),
	UINT64_C(0xe49b69c19ef14ad2), UINT64_C(0xefbe4786384f25e3), UINT64_C(0x0fc19dc68b8cd5b5), UINT64_C(0x240ca1cc77ac9c65),
	UINT64_C(0x2de92c6f592b0275), UINT64_C(0x4a7484aa6ea6e483), UINT64_C(0x5cb0a9dcbd41fbd4), UINT64_C(0x76f988da831153b5),
	UINT64_C(0x983e5152ee66dfab), UINT64_C(0xa831c66d2db43210), UINT64_C(0xb00327c898fb213f), UINT64_C(0xbf597fc7beef0ee4),
	UINT64_C(0xc6e00bf33da88fc2), UINT64_C(0xd5a79147930aa725), UINT64_C(0x06ca6351e003826f), UINT64_C(0x142929670a0e6e70),
	UINT64_C(0x27b70a8546d22ffc), UINT64_C(0x2e1b21385c26c926), UINT64_C(0x4d2c6dfc5ac42aed), UINT64_C(0x53380d139d95b3df),
	UINT64_C(0x650a73548baf63de), UINT64_C(0x766a0abb3c77b2a8), UINT64_C(0x81c2c92e47edaee6), UINT64_C(0x92722c851482353b),
	UINT64_C(0xa2bfe8a14cf10364), UINT64_C(0xa81a664bbc423001), UINT64_C(0xc24b8b70d0f89791), UINT64_C(0xc76c51a30654be30),
	UINT64_C(0xd192e819d6ef5218), UINT64_C(0xd69906245565a910), UINT64_C(0xf40e35855771202a), UINT64_C(0x106aa07032bbd1b8),
	UINT64_C(0x19a4c116b8d2d0c8), UINT64_C(0x1e376c085141ab53), UINT64_C(0x2748774cdf8eeb99), UINT64_C(0x34b0bcb5e19b48a8),
	UINT64_C(0x391c0cb3c5c95a63), UINT64_C(0x4ed8aa4ae3418acb), UINT64_C(0x5b9cca4f7763e373), UINT64_C(0x682e6ff3d6b2b8a3),
	UINT64_C(0x748f82ee5defb2fc), UINT64_C(0x78a5636f43172f60), UINT64_C(0x84c87814a1f0ab72), UINT64_C(0x8cc702081a6439ec),
	UINT64_C(0x90befffa23631e28), UINT64_C(0xa4506cebde82bde9), UINT64_C(0xbef9a3f7b2c67915), UINT64_C(0xc67178f2e372532b),
	UINT64_C(0xca273eceea26619c), UINT64_C(0xd186b8c721c0c207), UINT64_C(0xeada7dd6cde0eb1e), UINT64_C(0xf57d4f7fee6ed178),
	UINT64_C(0x06f067aa72176fba), UINT64_C(0x0a637dc5a2c898a6), UINT64_C(0x113f9804bef90dae), UINT64_C(0x1b710b35131c471b),
	UINT64_C(0x28db77f523047d84), UINT64_C(0x32caab7b40c72493), UINT64_C(0x3c9ebe0a15c9bebc), UINT64_C(0x431d67c49c100d4c),
	UINT64_C(0x4cc5d4becb3e42b6), UINT64_C(0x597f299cfc657e2a), UINT64_C(0x5fcb6fab3ad6faec), UINT64_C(0x6c44198c4a475817)
};

template <size_t Digest_Size>
void SHA512Base<Digest_Size>::update(const uint8_t (&block)[128]) {
	uint64_t words[80];
	for (size_t i = 0; i < 16; ++i) {
		words[i] = be64toh(reinterpret_cast<const uint64_t *>(block)[i]);
	}
	for (size_t i = 16; i < 80; ++i) {
		words[i] = words[i - 16] + (rotr(words[i - 15], 1) ^ rotr(words[i - 15], 8) ^ words[i - 15] >> 7) + words[i - 7] + (rotr(words[i - 2], 19) ^ rotr(words[i - 2], 61) ^ words[i - 2] >> 6);
	}
	uint64_t a = this->state[0], b = this->state[1], c = this->state[2], d = this->state[3], e = this->state[4], f = this->state[5], g = this->state[6], h = this->state[7];
	for (size_t i = 0; i < 80; ++i) {
		uint64_t t1 = h + ((g ^ f) & e ^ g) + (rotr(e, 14) ^ rotr(e, 18) ^ rotr(e, 41)) + sha512_round_constants[i] + words[i];
		h = g, g = f, f = e, e = d + t1;
		uint64_t t2 = (c & (b ^ a) ^ b & a) + (rotr(a, 28) ^ rotr(a, 34) ^ rotr(a, 39));
		d = c, c = b, b = a, a = t1 + t2;
	}
	this->state[0] += a, this->state[1] += b, this->state[2] += c, this->state[3] += d, this->state[4] += e, this->state[5] += f, this->state[6] += g, this->state[7] += h;
}


static const uint64_t sha512_init[8] = {
	UINT64_C(0x6a09e667f3bcc908), UINT64_C(0xbb67ae8584caa73b), UINT64_C(0x3c6ef372fe94f82b), UINT64_C(0xa54ff53a5f1d36f1),
	UINT64_C(0x510e527fade682d1), UINT64_C(0x9b05688c2b3e6c1f), UINT64_C(0x1f83d9abfb41bd6b), UINT64_C(0x5be0cd19137e2179)
};

SHA512::SHA512() : SHA512Base(sha512_init) {
}


static const uint64_t sha384_init[8] = {
	UINT64_C(0xcbbb9d5dc1059ed8), UINT64_C(0x629a292a367cd507), UINT64_C(0x9159015a3070dd17), UINT64_C(0x152fecd8f70e5939),
	UINT64_C(0x67332667ffc00b31), UINT64_C(0x8eb44a8768581511), UINT64_C(0xdb0c2e0d64f98fa7), UINT64_C(0x47b5481dbefa4fa4)
};

SHA384::SHA384() : SHA512Base(sha384_init) {
}


template class Hash<64, 20, 20, uint32_t, uint64_t, true>;
template class Hash<64, 32, 32, uint32_t, uint64_t, true>;
template class Hash<64, 32, 28, uint32_t, uint64_t, true>;
template class Hash<128, 64, 64, uint64_t, sha512_length_t, true>;
template class Hash<128, 64, 48, uint64_t, sha512_length_t, true>;


#include "hmac.tcc"

template class HMAC<SHA1>;
template class HMAC<SHA256>;
template class HMAC<SHA224>;
template class HMAC<SHA512>;
template class HMAC<SHA384>;
