#include <cstddef>

typedef void prf_t(const void *key, size_t key_len, const void *msg, size_t msg_len, void *out, size_t out_len);

template <typename Func>
void prf(const void *key, size_t key_len, const void *msg, size_t msg_len, void *out, size_t out_len);

void pbkdf2(prf_t *prf, size_t prf_out_len, const void *password, size_t password_len, const void *salt, size_t salt_len, size_t iterations, void *key, size_t key_len);
