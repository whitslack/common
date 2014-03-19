#include "compiler.h"
#include "mpn.h"

mp_limb_t * fp_add(mp_limb_t r[], const mp_limb_t n1[], const mp_limb_t n2[], const mp_limb_t p[], size_t l);

template <size_t L>
static inline mp_limb_t (& fp_add(mp_limb_t (&r)[L], const mp_limb_t (&n1)[L], const mp_limb_t (&n2)[L], const mp_limb_t (&p)[L]))[L] {
	return *reinterpret_cast<mp_limb_t (*)[L]>(fp_add(r, n1, n2, p, L));
}

mp_limb_t * fp_sub(mp_limb_t r[], const mp_limb_t n1[], const mp_limb_t n2[], const mp_limb_t p[], size_t l);

template <size_t L>
static inline mp_limb_t (& fp_sub(mp_limb_t (&r)[L], const mp_limb_t (&n1)[L], const mp_limb_t (&n2)[L], const mp_limb_t (&p)[L]))[L] {
	return *reinterpret_cast<mp_limb_t (*)[L]>(fp_sub(r, n1, n2, p, L));
}

mp_limb_t * fp_dbl(mp_limb_t r[], const mp_limb_t n[], const mp_limb_t p[], size_t l);

template <size_t L>
static inline mp_limb_t (& fp_dbl(mp_limb_t (&r)[L], const mp_limb_t (&n)[L], const mp_limb_t (&p)[L]))[L] {
	return *reinterpret_cast<mp_limb_t (*)[L]>(fp_dbl(r, n, p, L));
}

mp_limb_t * fp_mul(mp_limb_t * _restrict r, const mp_limb_t n1[], const mp_limb_t n2[], const mp_limb_t p[], size_t l);

template <size_t L>
static inline mp_limb_t (& fp_mul(mp_limb_t (& _restrict r)[L], const mp_limb_t (&n1)[L], const mp_limb_t (&n2)[L], const mp_limb_t (&p)[L]))[L] {
	return *reinterpret_cast<mp_limb_t (*)[L]>(fp_mul(r, n1, n2, p, L));
}

mp_limb_t * fp_mul_1(mp_limb_t * _restrict r, const mp_limb_t n1[], mp_limb_t n2, const mp_limb_t p[], size_t l);

template <size_t L>
static inline mp_limb_t (& fp_mul_1(mp_limb_t (& _restrict r)[L], const mp_limb_t (&n1)[L], mp_limb_t n2, const mp_limb_t (&p)[L]))[L] {
	return *reinterpret_cast<mp_limb_t (*)[L]>(fp_mul_1(r, n1, n2, p, L));
}

mp_limb_t * fp_sqr(mp_limb_t * _restrict r, const mp_limb_t n[], const mp_limb_t p[], size_t l);

template <size_t L>
static inline mp_limb_t (& fp_sqr(mp_limb_t (& _restrict r)[L], const mp_limb_t (&n)[L], const mp_limb_t (&p)[L]))[L] {
	return *reinterpret_cast<mp_limb_t (*)[L]>(fp_sqr(r, n, p, L));
}

mp_limb_t * fp_pow(mp_limb_t * _restrict r, const mp_limb_t n[], const mp_limb_t e[], const mp_limb_t p[], size_t l);

template <size_t L>
static inline mp_limb_t (& fp_pow(mp_limb_t (& _restrict r)[L], const mp_limb_t (&n)[L], const mp_limb_t (&e)[L], const mp_limb_t (&p)[L]))[L] {
	return *reinterpret_cast<mp_limb_t (*)[L]>(fp_pow(r, n, e, p, L));
}

mp_limb_t * fp_inv(mp_limb_t * _restrict r, const mp_limb_t n[], const mp_limb_t p[], size_t l);

template <size_t L>
static inline mp_limb_t (& fp_inv(mp_limb_t (& _restrict r)[L], const mp_limb_t (&n)[L], const mp_limb_t (&p)[L]))[L] {
	return *reinterpret_cast<mp_limb_t (*)[L]>(fp_inv(r, n, p, L));
}
