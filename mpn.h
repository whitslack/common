#pragma once

#include <cstddef>

#include <gmp.h>

#include "compiler.h"

#if GMP_LIMB_BITS == 64
#define MP_LIMB_C(lo, hi) (UINT64_C(lo) | UINT64_C(hi) << 32)
#elif GMP_LIMB_BITS == 32
#define MP_LIMB_C(lo, hi) UINT32_C(lo), UINT32_C(hi)
#else
#error "unsupported limb size"
#endif

#define MP_NLIMBS(n) (((n) + sizeof(mp_limb_t) - 1) / sizeof(mp_limb_t))

void bytes_to_mpn(mp_limb_t mpn[], const std::byte bytes[], size_t n) noexcept;

template <size_t N>
static inline void bytes_to_mpn(mp_limb_t (&mpn)[MP_NLIMBS(N)], const std::byte (&bytes)[N]) noexcept {
	bytes_to_mpn(mpn, bytes, N);
}

void mpn_to_bytes(std::byte bytes[], const mp_limb_t mpn[], size_t n) noexcept;

template <size_t N>
static inline void mpn_to_bytes(std::byte (&bytes)[N], const mp_limb_t (&mpn)[MP_NLIMBS(N)]) noexcept {
	mpn_to_bytes(bytes, mpn, N);
}

#if __GNU_MP__ < 5
#include <cstring>
static inline void mpn_copyi(mp_limb_t *rp, const mp_limb_t *s1p, mp_size_t n) noexcept {
	std::memcpy(rp, s1p, n * sizeof(mp_limb_t));
}
static inline void mpn_zero(mp_limb_t *rp, mp_size_t n) noexcept {
	std::memset(rp, 0, n * sizeof(mp_limb_t));
}
#endif

#if __GNU_MP_RELEASE < 60100
static inline int _pure mpn_zero_p(const mp_limb_t *sp, mp_size_t n) noexcept {
	for (mp_size_t i = 0; i < n; ++i) {
		if (sp[i] != 0) {
			return 0;
		}
	}
	return 1;
}
#endif

static inline bool _pure mpn_one_p(const mp_limb_t n[], size_t l) noexcept {
	return l > 0 && n[0] == 1 && mpn_zero_p(n + 1, l - 1);
}

static inline bool _pure mpn_even_p(const mp_limb_t n[], size_t l) noexcept {
	return l == 0 || (n[0] & 1) == 0;
}
