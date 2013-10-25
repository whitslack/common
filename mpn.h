#pragma once

#include <cstddef>
#include <cstdint>

#include <gmp.h>

#if GMP_LIMB_BITS == 64
#define MP_LIMB_C(lo, hi) (UINT64_C(lo) | UINT64_C(hi) << 32)
#elif GMP_LIMB_BITS == 32
#define MP_LIMB_C(lo, hi) UINT32_C(lo), UINT32_C(hi)
#else
#error "unsupported limb size"
#endif

#define MP_NLIMBS(n) (((n) + sizeof(mp_limb_t) - 1) / sizeof(mp_limb_t))

void bytes_to_mpn(mp_limb_t mpn[], const uint8_t bytes[], size_t n);

template <size_t N>
static inline void bytes_to_mpn(mp_limb_t (&mpn)[MP_NLIMBS(N)], const uint8_t (&bytes)[N]) {
	bytes_to_mpn(mpn, bytes, N);
}

void mpn_to_bytes(uint8_t bytes[], const mp_limb_t mpn[], size_t n);

template <size_t N>
static inline void mpn_to_bytes(uint8_t (&bytes)[N], const mp_limb_t (&mpn)[MP_NLIMBS(N)]) {
	mpn_to_bytes(bytes, mpn, N);
}

#if __GNU_MP__ < 5
#include <cstring>
static inline void mpn_copyi(mp_limb_t *rp, const mp_limb_t *s1p, mp_size_t n) {
	std::memcpy(rp, s1p, n * sizeof(mp_limb_t));
}
static inline void mpn_zero(mp_limb_t *rp, mp_size_t n) {
	std::memset(rp, 0, n * sizeof(mp_limb_t));
}
#endif

static inline bool mpn_zero_p(const mp_limb_t n[], size_t l) {
	for (size_t i = 0; i < l; ++i) {
		if (n[i] != 0) {
			return false;
		}
	}
	return true;
}

static inline bool mpn_one_p(const mp_limb_t n[], size_t l) {
	return n[0] == 1 && (--l == 0 || mpn_zero_p(n + 1, l));
}

static inline bool mpn_even_p(const mp_limb_t n[]) {
	return (n[0] & 1) == 0;
}
