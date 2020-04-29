#include "hmac.h"


template <typename Hash_Type>
constexpr size_t HMAC<Hash_Type>::digest_size;

template <typename Hash_Type>
HMAC<Hash_Type>::HMAC(const void *key, size_t n) {
	if (n > sizeof secret) {
		hash.write_fully(key, n);
		std::memcpy(secret, hash.digest().data(), n = std::min(hash_type::digest_size, sizeof secret));
		hash = hash_type();
	}
	else {
		std::memcpy(secret, key, n);
	}
	if (n < hash_type::block_size) {
		std::memset(secret + n, 0, sizeof secret - n);
	}
	for (size_t i = 0; i < sizeof secret; ++i) {
		secret[i] ^= static_cast<std::byte>(0x36);
	}
	hash.write_fully(secret, sizeof secret);
}

template <typename Hash_Type>
const typename HMAC<Hash_Type>::digest_type & HMAC<Hash_Type>::digest() {
	auto ihash = hash.digest();
	hash = hash_type();
	for (size_t i = 0; i < sizeof secret; ++i) {
		secret[i] ^= static_cast<std::byte>(0x36 ^ 0x5c);
	}
	hash.write_fully(secret, sizeof secret);
	hash.write_fully(ihash.data(), ihash.size());
	return hash.digest();
}
