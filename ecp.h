#include "compiler.h"
#include "mpn.h"

extern const mp_limb_t secp224k1_p[MP_NLIMBS(29)], secp224k1_a[MP_NLIMBS(29)], secp224k1_G[3][MP_NLIMBS(29)], secp224k1_n[MP_NLIMBS(29)];

extern const mp_limb_t secp256k1_p[MP_NLIMBS(32)], secp256k1_a[MP_NLIMBS(32)], secp256k1_G[3][MP_NLIMBS(32)], secp256k1_n[MP_NLIMBS(32)];

static inline mp_limb_t * ecp_copy(mp_limb_t R[], const mp_limb_t N[], size_t l) noexcept {
	mpn_copyi(&R[0], &N[0], l), mpn_copyi(&R[l], &N[l], l), mpn_copyi(&R[l * 2], &N[l * 2], l);
	return R;
}

template <size_t L>
static inline mp_limb_t (& ecp_copy(mp_limb_t (&R)[3][L], const mp_limb_t (&N)[3][L]) noexcept)[3][L] {
	return *reinterpret_cast<mp_limb_t (*)[3][L]>(ecp_copy(*R, *N, L));
}

mp_limb_t * ecp_dbl(mp_limb_t * _restrict R, const mp_limb_t N[], const mp_limb_t a[], const mp_limb_t p[], size_t l) noexcept;

template <size_t L>
static inline mp_limb_t (& ecp_dbl(mp_limb_t (& _restrict R)[3][L], const mp_limb_t (&N)[3][L], const mp_limb_t (&a)[L], const mp_limb_t (&p)[L]) noexcept)[3][L] {
	return *reinterpret_cast<mp_limb_t (*)[3][L]>(ecp_dbl(*R, *N, a, p, L));
}

mp_limb_t * ecp_add(mp_limb_t * _restrict R, const mp_limb_t N1[], const mp_limb_t N2[], const mp_limb_t a[], const mp_limb_t p[], size_t l) noexcept;

template <size_t L>
static inline mp_limb_t (& ecp_add(mp_limb_t (& _restrict R)[3][L], const mp_limb_t (&N1)[3][L], const mp_limb_t (&N2)[3][L], const mp_limb_t (&a)[L], const mp_limb_t (&p)[L]) noexcept)[3][L] {
	return *reinterpret_cast<mp_limb_t (*)[3][L]>(ecp_add(*R, *N1, *N2, a, p, L));
}

mp_limb_t * ecp_mul(mp_limb_t * _restrict R, const mp_limb_t n1[], const mp_limb_t N2[], const mp_limb_t a[], const mp_limb_t p[], size_t l) noexcept;

template <size_t L>
static inline mp_limb_t (& ecp_mul(mp_limb_t (& _restrict R)[3][L], const mp_limb_t (&n1)[L], const mp_limb_t (&N2)[3][L], const mp_limb_t (&a)[L], const mp_limb_t (&p)[L]) noexcept)[3][L] {
	return *reinterpret_cast<mp_limb_t (*)[3][L]>(ecp_mul(*R, n1, *N2, a, p, L));
}

mp_limb_t * ecp_proj(mp_limb_t * _restrict R, const mp_limb_t N[], const mp_limb_t p[], size_t l);

template <size_t L>
static inline mp_limb_t (& ecp_proj(mp_limb_t (& _restrict R)[3][L], const mp_limb_t (&N)[3][L], const mp_limb_t (&p)[L]))[3][L] {
	return *reinterpret_cast<mp_limb_t (*)[3][L]>(ecp_proj(*R, *N, p, L));
}

void ecp_pubkey(mp_limb_t * _restrict Q, const mp_limb_t p[], const mp_limb_t a[], const mp_limb_t G[], const mp_limb_t d[], size_t l);

template <size_t L>
static inline void ecp_pubkey(mp_limb_t (& _restrict Q)[3][L], const mp_limb_t (&p)[L], const mp_limb_t (&a)[L], const mp_limb_t (&G)[3][L], const mp_limb_t (&d)[L]) {
	ecp_pubkey(*Q, p, a, *G, d, L);
}

void ecp_sign(mp_limb_t * _restrict r, mp_limb_t * _restrict s, const mp_limb_t p[], const mp_limb_t a[], const mp_limb_t G[], const mp_limb_t n[], const mp_limb_t d[], const mp_limb_t z[], size_t l);

template <size_t L>
static inline void ecp_sign(mp_limb_t (& _restrict r)[L], mp_limb_t (& _restrict s)[L], const mp_limb_t (&p)[L], const mp_limb_t (&a)[L], const mp_limb_t (&G)[3][L], const mp_limb_t (&n)[L], const mp_limb_t (&d)[L], const mp_limb_t (&z)[L]) {
	ecp_sign(r, s, p, a, *G, n, d, z, L);
}

bool ecp_verify(const mp_limb_t p[], const mp_limb_t a[], const mp_limb_t G[], const mp_limb_t n[], const mp_limb_t Q[], const mp_limb_t z[], const mp_limb_t r[], const mp_limb_t s[], size_t l) _pure;

template <size_t L>
static inline bool _pure ecp_verify(const mp_limb_t (&p)[L], const mp_limb_t (&a)[L], const mp_limb_t (&G)[3][L], const mp_limb_t (&n)[L], const mp_limb_t (&Q)[3][L], const mp_limb_t (&z)[L], const mp_limb_t (&r)[L], const mp_limb_t (&s)[L]) {
	return ecp_verify(p, a, *G, n, *Q, z, r, s, L);
}
