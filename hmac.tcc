#include "hmac.h"


template <typename Hash_Type>
constexpr size_t HMAC<Hash_Type>::digest_size;

template <typename Hash_Type>
HMAC<Hash_Type>::HMAC(const void *key, size_t n) {
	if (n > sizeof secret) {
		hash.write(key, n);
		std::memcpy(secret, hash.digest(), n = std::min(hash_type::digest_size, sizeof secret));
		hash = hash_type();
	}
	else {
		std::memcpy(secret, key, n);
	}
	if (n < hash_type::block_size) {
		std::memset(secret + n, 0, sizeof secret - n);
	}
	for (size_t i = 0; i < sizeof secret; ++i) {
		secret[i] ^= 0x36;
	}
	hash.write(secret, sizeof secret, true);
}

template <typename Hash_Type>
const uint8_t (& HMAC<Hash_Type>::digest())[digest_size] {
	uint8_t ihash[hash_type::digest_size];
	std::memcpy(ihash, hash.digest(), sizeof ihash);
	hash = hash_type();
	for (size_t i = 0; i < sizeof secret; ++i) {
		secret[i] ^= 0x36 ^ 0x5c;
	}
	hash.write(secret, sizeof secret, true);
	hash.write(ihash, sizeof ihash, false);
	return hash.digest();
}
