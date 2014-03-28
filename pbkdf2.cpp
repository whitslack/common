#include "pbkdf2.h"

#include <cassert>

#include "hmac.h"
#include "sha.h"

template <typename Func>
void prf(const void *key, size_t key_len, const void *msg, size_t msg_len, void *out, size_t out_len _unused) {
	assert(out_len == Func::digest_size);
	Func func(key, key_len);
	func.write(msg, msg_len);
	std::memcpy(out, func.digest(), Func::digest_size);
}

template void prf<HMAC<SHA1>>(const void *, size_t, const void *, size_t, void *, size_t);
template void prf<HMAC<SHA256>>(const void *, size_t, const void *, size_t, void *, size_t);
template void prf<HMAC<SHA224>>(const void *, size_t, const void *, size_t, void *, size_t);
template void prf<HMAC<SHA512>>(const void *, size_t, const void *, size_t, void *, size_t);
template void prf<HMAC<SHA384>>(const void *, size_t, const void *, size_t, void *, size_t);

void pbkdf2(prf_t *prf, size_t prf_out_len, const void *password, size_t password_len, const void *salt, size_t salt_len, size_t c, void *key, size_t key_len) {
	size_t i = 0;
	uint8_t salt_ext[salt_len + sizeof(uint32_t)], f[prf_out_len], f1[prf_out_len];
	std::memcpy(salt_ext, salt, salt_len);
	while (key_len > 0) {
		++i;
		salt_ext[salt_len + 0] = static_cast<uint8_t>(i >> 24);
		salt_ext[salt_len + 1] = static_cast<uint8_t>(i >> 16);
		salt_ext[salt_len + 2] = static_cast<uint8_t>(i >>  8);
		salt_ext[salt_len + 3] = static_cast<uint8_t>(i >>  0);
		(*prf)(password, password_len, salt_ext, sizeof salt_ext, f1, prf_out_len);
		std::memcpy(f, f1, sizeof f);
		for (size_t j = 1; j < c; ++j) {
			(*prf)(password, password_len, f1, prf_out_len, f1, prf_out_len);
			for (size_t k = 0; k < prf_out_len; ++k) {
				f[k] ^= f1[k];
			}
		}
		size_t n = std::min(key_len, prf_out_len);
		std::memcpy(key, f, n);
		key = static_cast<uint8_t *>(key) + n, key_len -= n;
	}
}
