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

template <size_t n>
static inline void bytes_to_mpn(mp_limb_t (&mpn)[MP_NLIMBS(n)], const uint8_t (&bytes)[n]) {
	bytes_to_mpn(mpn, bytes, n);
}

void mpn_to_bytes(uint8_t bytes[], const mp_limb_t mpn[], size_t n);

template <size_t n>
static inline void mpn_to_bytes(uint8_t (&bytes)[n], const mp_limb_t (&mpn)[MP_NLIMBS(n)]) {
	mpn_to_bytes(bytes, mpn, n);
}

extern const mp_limb_t secp224k1_p[MP_NLIMBS(29)], secp224k1_a[MP_NLIMBS(29)], secp224k1_G[3][MP_NLIMBS(29)], secp224k1_n[MP_NLIMBS(29)];

extern const mp_limb_t secp256k1_p[MP_NLIMBS(32)], secp256k1_a[MP_NLIMBS(32)], secp256k1_G[3][MP_NLIMBS(32)], secp256k1_n[MP_NLIMBS(32)];

void ecp_pubkey(mp_limb_t Q[], const mp_limb_t p[], const mp_limb_t a[], const mp_limb_t G[], const mp_limb_t d[], size_t l);

template <size_t l>
static inline void ecp_pubkey(mp_limb_t (&Q)[3][l], const mp_limb_t (&p)[l], const mp_limb_t (&a)[l], const mp_limb_t (&G)[3][l], const mp_limb_t (&d)[l]) {
	ecp_pubkey(*Q, p, a, *G, d, l);
}

void ecp_sign(mp_limb_t r[], mp_limb_t s[], const mp_limb_t p[], const mp_limb_t a[], const mp_limb_t G[], const mp_limb_t n[], const mp_limb_t d[], const mp_limb_t z[], size_t l);

template <size_t l>
static inline void ecp_sign(mp_limb_t (&r)[l], mp_limb_t (&s)[l], const mp_limb_t (&p)[l], const mp_limb_t (&a)[l], const mp_limb_t (&G)[3][l], const mp_limb_t (&n)[l], const mp_limb_t (&d)[l], const mp_limb_t (&z)[l]) {
	ecp_sign(r, s, p, a, *G, n, d, z, l);
}

bool ecp_verify(const mp_limb_t p[], const mp_limb_t a[], const mp_limb_t G[], const mp_limb_t n[], const mp_limb_t Q[], const mp_limb_t z[], const mp_limb_t r[], const mp_limb_t s[], size_t l);

template <size_t l>
static inline bool ecp_verify(const mp_limb_t (&p)[l], const mp_limb_t (&a)[l], const mp_limb_t (&G)[3][l], const mp_limb_t (&n)[l], const mp_limb_t (&Q)[3][l], const mp_limb_t (&z)[l], const mp_limb_t (&r)[l], const mp_limb_t (&s)[l]) {
	return ecp_verify(p, a, *G, n, *Q, z, r, s, l);
}
