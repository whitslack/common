#include <cstddef>

void scrypt(void *out, size_t out_len, const void *pass, size_t pass_len, const void *salt, size_t salt_len, size_t r, size_t n, size_t p);
