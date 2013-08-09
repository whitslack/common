#include "ecp.h"

#include <random>

#if __GNU_MP__ < 5
#include <cstring>
static void mpn_copyi(mp_limb_t *rp, const mp_limb_t *s1p, mp_size_t n) {
	std::memcpy(rp, s1p, n * sizeof(mp_limb_t));
}
static void mpn_zero(mp_limb_t *rp, mp_size_t n) {
	std::memset(rp, 0, n * sizeof(mp_limb_t));
}
#endif

void bytes_to_mpn(mp_limb_t mpn[], const uint8_t bytes[], size_t n) {
	while (n > sizeof(mp_limb_t)) {
		mp_limb_t limb = 0;
#if GMP_LIMB_BITS >= 64
		limb |= static_cast<mp_limb_t>(bytes[n - 8]) << 56;
		limb |= static_cast<mp_limb_t>(bytes[n - 7]) << 48;
		limb |= static_cast<mp_limb_t>(bytes[n - 6]) << 40;
		limb |= static_cast<mp_limb_t>(bytes[n - 5]) << 32;
#endif
#if GMP_LIMB_BITS >= 32
		limb |= static_cast<mp_limb_t>(bytes[n - 4]) << 24;
		limb |= static_cast<mp_limb_t>(bytes[n - 3]) << 16;
		limb |= static_cast<mp_limb_t>(bytes[n - 2]) << 8;
		limb |= static_cast<mp_limb_t>(bytes[n - 1]);
#endif
		*mpn++ = limb;
		n -= sizeof(mp_limb_t);
	}
	mp_limb_t limb = 0;
	switch (n) {
#if GMP_LIMB_BITS >= 64
		case 8:
			limb |= static_cast<mp_limb_t>(bytes[n - 8]) << 56;
		case 7:
			limb |= static_cast<mp_limb_t>(bytes[n - 7]) << 48;
		case 6:
			limb |= static_cast<mp_limb_t>(bytes[n - 6]) << 40;
		case 5:
			limb |= static_cast<mp_limb_t>(bytes[n - 5]) << 32;
#endif
#if GMP_LIMB_BITS >= 32
		case 4:
			limb |= static_cast<mp_limb_t>(bytes[n - 4]) << 24;
		case 3:
			limb |= static_cast<mp_limb_t>(bytes[n - 3]) << 16;
		case 2:
			limb |= static_cast<mp_limb_t>(bytes[n - 2]) << 8;
		case 1:
			limb |= static_cast<mp_limb_t>(bytes[n - 1]);
#endif
	}
	*mpn = limb;
}

void mpn_to_bytes(uint8_t bytes[], const mp_limb_t mpn[], size_t n) {
	while (n > sizeof(mp_limb_t)) {
		mp_limb_t limb = *mpn++;
#if GMP_LIMB_BITS >= 64
		bytes[n - 8] = static_cast<uint8_t>(limb >> 56);
		bytes[n - 7] = static_cast<uint8_t>(limb >> 48);
		bytes[n - 6] = static_cast<uint8_t>(limb >> 40);
		bytes[n - 5] = static_cast<uint8_t>(limb >> 32);
#endif
#if GMP_LIMB_BITS >= 32
		bytes[n - 4] = static_cast<uint8_t>(limb >> 24);
		bytes[n - 3] = static_cast<uint8_t>(limb >> 16);
		bytes[n - 2] = static_cast<uint8_t>(limb >> 8);
		bytes[n - 1] = static_cast<uint8_t>(limb);
#endif
		n -= sizeof(mp_limb_t);
	}
	mp_limb_t limb = *mpn;
	switch (n) {
#if GMP_LIMB_BITS >= 64
		case 8:
			bytes[n - 8] = static_cast<uint8_t>(limb >> 56);
		case 7:
			bytes[n - 7] = static_cast<uint8_t>(limb >> 48);
		case 6:
			bytes[n - 6] = static_cast<uint8_t>(limb >> 40);
		case 5:
			bytes[n - 5] = static_cast<uint8_t>(limb >> 32);
#endif
#if GMP_LIMB_BITS >= 32
		case 4:
			bytes[n - 4] = static_cast<uint8_t>(limb >> 24);
		case 3:
			bytes[n - 3] = static_cast<uint8_t>(limb >> 16);
		case 2:
			bytes[n - 2] = static_cast<uint8_t>(limb >> 8);
		case 1:
			bytes[n - 1] = static_cast<uint8_t>(limb);
#endif
	}
}

const mp_limb_t secp224k1_p[MP_NLIMBS(29)] = {
	MP_LIMB_C(0xFFFFE56D, 0xFFFFFFFE), MP_LIMB_C(0xFFFFFFFF, 0xFFFFFFFF),
	MP_LIMB_C(0xFFFFFFFF, 0xFFFFFFFF), MP_LIMB_C(0xFFFFFFFF, 0x00)
};
const mp_limb_t secp224k1_a[MP_NLIMBS(29)] = {
	MP_LIMB_C(0x00000000, 0x00000000), MP_LIMB_C(0x00000000, 0x00000000),
	MP_LIMB_C(0x00000000, 0x00000000), MP_LIMB_C(0x00000000, 0x00)
};
const mp_limb_t secp224k1_G[3][MP_NLIMBS(29)] = {
	{
		MP_LIMB_C(0xB6B7A45C, 0x0F7E650E), MP_LIMB_C(0xE47075A9, 0x69A467E9),
		MP_LIMB_C(0x30FC28A1, 0x4DF099DF), MP_LIMB_C(0xA1455B33, 0x00)
	},
	{
		MP_LIMB_C(0x556D61A5, 0xE2CA4BDB), MP_LIMB_C(0xC0B0BD59, 0xF7E319F7),
		MP_LIMB_C(0x82CAFBD6, 0x7FBA3442), MP_LIMB_C(0x7E089FED, 0x00)
	},
	{
		MP_LIMB_C(0x00000001, 0x00000000), MP_LIMB_C(0x00000000, 0x00000000),
		MP_LIMB_C(0x00000000, 0x00000000), MP_LIMB_C(0x00000000, 0x00)
	}
};
const mp_limb_t secp224k1_n[MP_NLIMBS(29)] = {
	MP_LIMB_C(0x769FB1F7, 0xCAF0A971), MP_LIMB_C(0xD2EC6184, 0x0001DCE8),
	MP_LIMB_C(0x00000000, 0x00000000), MP_LIMB_C(0x00000000, 0x01)
};

const mp_limb_t secp256k1_p[MP_NLIMBS(32)] = {
	MP_LIMB_C(0xFFFFFC2F, 0xFFFFFFFE), MP_LIMB_C(0xFFFFFFFF, 0xFFFFFFFF),
	MP_LIMB_C(0xFFFFFFFF, 0xFFFFFFFF), MP_LIMB_C(0xFFFFFFFF, 0xFFFFFFFF)
};
const mp_limb_t secp256k1_a[MP_NLIMBS(32)] = {
	MP_LIMB_C(0x00000000, 0x00000000), MP_LIMB_C(0x00000000, 0x00000000),
	MP_LIMB_C(0x00000000, 0x00000000), MP_LIMB_C(0x00000000, 0x00000000)
};
const mp_limb_t secp256k1_G[3][MP_NLIMBS(32)] = {
	{
		MP_LIMB_C(0x16F81798, 0x59F2815B), MP_LIMB_C(0x2DCE28D9, 0x029BFCDB),
		MP_LIMB_C(0xCE870B07, 0x55A06295), MP_LIMB_C(0xF9DCBBAC, 0x79BE667E)
	},
	{
		MP_LIMB_C(0xFB10D4B8, 0x9C47D08F), MP_LIMB_C(0xA6855419, 0xFD17B448),
		MP_LIMB_C(0x0E1108A8, 0x5DA4FBFC), MP_LIMB_C(0x26A3C465, 0x483ADA77)
	},
	{
		MP_LIMB_C(0x00000001, 0x00000000), MP_LIMB_C(0x00000000, 0x00000000),
		MP_LIMB_C(0x00000000, 0x00000000), MP_LIMB_C(0x00000000, 0x00000000)
	}
};
const mp_limb_t secp256k1_n[MP_NLIMBS(32)] = {
	MP_LIMB_C(0xD0364141, 0xBFD25E8C), MP_LIMB_C(0xAF48A03B, 0xBAAEDCE6),
	MP_LIMB_C(0xFFFFFFFE, 0xFFFFFFFF), MP_LIMB_C(0xFFFFFFFF, 0xFFFFFFFF)
};

static bool mpn_zero_p(const mp_limb_t n[], size_t l) {
	for (size_t i = 0; i < l; ++i) {
		if (n[i] != 0) {
			return false;
		}
	}
	return true;
}

static bool mpn_one_p(const mp_limb_t n[], size_t l) {
	return n[0] == 1 && (--l == 0 || mpn_zero_p(n + 1, l));
}

static bool mpn_even_p(const mp_limb_t n[]) {
	return (n[0] & 1) == 0;
}

static mp_limb_t * fp_add(mp_limb_t r[], const mp_limb_t n1[], const mp_limb_t n2[], const mp_limb_t p[], size_t l) {
	if (mpn_add_n(r, n1, n2, l) || mpn_cmp(r, p, l) >= 0) {
		mpn_sub_n(r, r, p, l);
	}
	return r;
}

static mp_limb_t * fp_sub(mp_limb_t r[], const mp_limb_t n1[], const mp_limb_t n2[], const mp_limb_t p[], size_t l) {
	if (mpn_sub_n(r, n1, n2, l)) {
		mpn_add_n(r, r, p, l);
	}
	return r;
}

static mp_limb_t * fp_dbl(mp_limb_t r[], const mp_limb_t n[], const mp_limb_t p[], size_t l) {
	if (mpn_lshift(r, n, l, 1) || mpn_cmp(r, p, l) >= 0) {
		mpn_sub_n(r, r, p, l);
	}
	return r;
}

static mp_limb_t * fp_mul(mp_limb_t r[], const mp_limb_t n1[], const mp_limb_t n2[], const mp_limb_t p[], size_t l) {
	mpn_zero(r, l);
	bool active = false;
	for (size_t i = l; i > 0;) {
		mp_limb_t w = n2[--i];
		for (size_t j = sizeof(mp_limb_t) * 8; j > 0; --j) {
			if (active) {
				fp_dbl(r, r, p, l);
			}
			if (static_cast<mp_limb_signed_t>(w) < 0) {
				fp_add(r, r, n1, p, l);
				active = true;
			}
			w <<= 1;
		}
	}
	return r;
}

static mp_limb_t * fp_sqr(mp_limb_t r[], const mp_limb_t n[], const mp_limb_t p[], size_t l) {
	return fp_mul(r, n, n, p, l);
}

static mp_limb_t * fp_inv(mp_limb_t r[], const mp_limb_t n[], const mp_limb_t p[], size_t l) {
	mp_limb_t u[l], v[l], s[l];
	mpn_copyi(u, n, l), mpn_copyi(v, p, l);
	mpn_zero(r, l), mpn_zero(s, l);
	r[0] = 1;
	for (;;) {
		if (mpn_one_p(u, l)) {
			return r;
		}
		if (mpn_one_p(v, l)) {
			mpn_copyi(r, s, l);
			return r;
		}
		while (mpn_even_p(u)) {
			mpn_rshift(u, u, l, 1);
			if (mpn_even_p(r)) {
				mpn_rshift(r, r, l, 1);
			}
			else {
				mp_limb_t c = mpn_add_n(r, r, p, l) << sizeof(mp_limb_t) * 8 - 1;
				mpn_rshift(r, r, l, 1);
				r[l - 1] |= c;
			}
		}
		while (mpn_even_p(v)) {
			mpn_rshift(v, v, l, 1);
			if (mpn_even_p(s)) {
				mpn_rshift(s, s, l, 1);
			}
			else {
				mp_limb_t c = mpn_add_n(s, s, p, l) << sizeof(mp_limb_t) * 8 - 1;
				mpn_rshift(s, s, l, 1);
				s[l - 1] |= c;
			}
		}
		if (mpn_cmp(u, v, l) >= 0) {
			mpn_sub_n(u, u, v, l);
			fp_sub(r, r, s, p, l);
		}
		else {
			mpn_sub_n(v, v, u, l);
			fp_sub(s, s, r, p, l);
		}
	}
}

static mp_limb_t * ecp_copy(mp_limb_t R[], const mp_limb_t N[], size_t l) {
	mpn_copyi(&R[0], &N[0], l), mpn_copyi(&R[l], &N[l], l), mpn_copyi(&R[l * 2], &N[l * 2], l);
	return R;
}

static mp_limb_t * ecp_dbl(mp_limb_t R[], const mp_limb_t N[], const mp_limb_t a[], const mp_limb_t p[], size_t l) {
	const mp_limb_t *x = &N[0], *y = &N[l], *z = &N[l * 2];
	mp_limb_t *xr = &R[0], *yr = &R[l], *zr = &R[l * 2];
	mp_limb_t t0[l], t1[l], t2[l], t3[l];
	fp_add(t0, t0, fp_dbl(t1, fp_sqr(t0, x, p, l), p, l), p, l);
	if (!mpn_zero_p(a, l)) {
		fp_add(t0, t0, fp_mul(t1, a, fp_sqr(t2, fp_sqr(t1, z, p, l), p, l), p, l), p, l);
	}
	fp_dbl(t1, fp_sqr(t1, y, p, l), p, l);
	fp_dbl(t2, fp_mul(t2, x, t1, p, l), p, l);
	fp_dbl(t3, fp_sqr(t3, t1, p, l), p, l);
	fp_sub(xr, fp_sqr(xr, t0, p, l), fp_dbl(t1, t2, p, l), p, l);
	fp_sub(yr, fp_mul(yr, t0, fp_sub(t1, t2, xr, p, l), p, l), t3, p, l);
	fp_dbl(zr, fp_mul(zr, y, z, p, l), p, l);
	return R;
}

static mp_limb_t * ecp_add_aff(mp_limb_t R[], const mp_limb_t N1[], const mp_limb_t N2[], const mp_limb_t a[], const mp_limb_t p[], size_t l) {
	const mp_limb_t *x1 = &N1[0], *y1 = &N1[l], *z1 = &N1[l * 2], *x2 = &N2[0], *y2 = &N2[l];
	mp_limb_t *xr = &R[0], *yr = &R[l], *zr = &R[l * 2];
	mp_limb_t t0[l], t1[l], t2[l], t3[l], t4[l];
	fp_sqr(t0, z1, p, l);
	fp_mul(t1, x2, t0, p, l);
	fp_mul(t2, z1, t0, p, l);
	fp_mul(t0, y2, t2, p, l);
	if (mpn_cmp(t1, x1, l) == 0) {
		if (mpn_cmp(t0, y1, l) == 0) {
			return ecp_dbl(R, N1, a, p, l);
		}
		mpn_zero(xr, l), mpn_zero(yr, l), mpn_zero(zr, l);
		xr[0] = yr[0] = 1;
		return R;
	}
	fp_sub(t2, t1, x1, p, l);
	fp_sub(t1, t0, y1, p, l);
	fp_sqr(t0, t2, p, l);
	fp_mul(t3, t0, t2, p, l);
	fp_mul(t4, x1, t0, p, l);
	fp_sub(xr, fp_sub(xr, fp_sqr(xr, t1, p, l), t3, p, l), fp_dbl(t0, t4, p, l), p, l);
	fp_sub(yr, fp_mul(yr, t1, fp_sub(t4, t4, xr, p, l), p, l), fp_mul(t0, y1, t3, p, l), p, l);
	fp_mul(zr, z1, t2, p, l);
	return R;
}

static mp_limb_t * ecp_add(mp_limb_t R[], const mp_limb_t N1[], const mp_limb_t N2[], const mp_limb_t a[], const mp_limb_t p[], size_t l) {
	const mp_limb_t *z2 = &N2[l * 2];
	if (mpn_one_p(z2, l)) {
		return ecp_add_aff(R, N1, N2, a, p, l);
	}
	const mp_limb_t *x1 = &N1[0], *y1 = &N1[l], *z1 = &N1[l * 2], *x2 = &N2[0], *y2 = &N2[l];
	mp_limb_t *xr = &R[0], *yr = &R[l], *zr = &R[l * 2];
	mp_limb_t t0[l], t1[l], t2[l], t3[l], t4[l], t5[l], t6[l];
	fp_sqr(t0, z1, p, l);
	fp_mul(t1, x2, t0, p, l);
	fp_mul(t2, z1, t0, p, l);
	fp_mul(t0, y2, t2, p, l);
	fp_sqr(t2, z2, p, l);
	fp_mul(t3, x1, t2, p, l);
	fp_mul(t4, z2, t2, p, l);
	fp_mul(t2, y1, t4, p, l);
	if (mpn_cmp(t3, t1, l) == 0) {
		if (mpn_cmp(t2, t0, l) == 0) {
			return ecp_dbl(R, N1, a, p, l);
		}
		mpn_zero(xr, l), mpn_zero(yr, l), mpn_zero(zr, l);
		xr[0] = yr[0] = 1;
		return R;
	}
	fp_sub(t4, t1, t3, p, l);
	fp_sub(t1, t0, t2, p, l);
	fp_sqr(t0, t4, p, l);
	fp_mul(t5, t4, t0, p, l);
	fp_mul(t6, t3, t0, p, l);
	fp_sub(xr, fp_sub(xr, fp_sqr(xr, t1, p, l), t5, p, l), fp_dbl(t0, t6, p, l), p, l);
	fp_sub(yr, fp_mul(yr, t1, fp_sub(t6, t6, xr, p, l), p, l), fp_mul(t0, t2, t5, p, l), p, l);
	fp_mul(zr, t4, fp_mul(t0, z1, z2, p, l), p, l);
	return R;
}

static mp_limb_t * ecp_mul_(mp_limb_t R[], const mp_limb_t n1[], const mp_limb_t N2[], const mp_limb_t a[], const mp_limb_t p[], size_t l, mp_limb_t * (*add)(mp_limb_t [], const mp_limb_t [], const mp_limb_t [], const mp_limb_t [], const mp_limb_t [], size_t)) {
	bool active = false;
	size_t swaps = 0;
	mp_limb_t Ss[l * 3], *S = Ss, *T;
	for (size_t i = l; i > 0;) {
		mp_limb_t w = n1[--i];
		for (size_t j = sizeof(mp_limb_t) * 8; j > 0; --j) {
			if (active) {
				ecp_dbl(S, R, a, p, l);
				T = S, S = R, R = T, ++swaps;
			}
			if (static_cast<mp_limb_signed_t>(w) < 0) {
				if (active) {
					(*add)(S, R, N2, a, p, l);
					T = S, S = R, R = T, ++swaps;
				}
				else {
					ecp_copy(R, N2, l);
					active = true;
				}
			}
			w <<= 1;
		}
	}
	if (swaps & 1) {
		return ecp_copy(S, R, l);
	}
	return R;
}

static mp_limb_t * ecp_mul(mp_limb_t R[], const mp_limb_t n1[], const mp_limb_t N2[], const mp_limb_t a[], const mp_limb_t p[], size_t l) {
	return ecp_mul_(R, n1, N2, a, p, l, mpn_one_p(&N2[l * 2], l) ? &ecp_add_aff : &ecp_add);
}

static mp_limb_t * ecp_proj(mp_limb_t R[], const mp_limb_t N[], const mp_limb_t p[], size_t l) {
	const mp_limb_t *x = &N[0], *y = &N[l], *z = &N[l * 2];
	mp_limb_t *xr = &R[0], *yr = &R[l], *zr = &R[l * 2];
	mp_limb_t t0[l], t1[l], t2[l];
	fp_mul(t2, t0, fp_sqr(t1, fp_inv(t0, z, p, l), p, l), p, l);
	fp_mul(xr, x, t1, p, l);
	fp_mul(yr, y, t2, p, l);
	mpn_zero(zr, l), zr[0] = 1;
	return R;
}

void ecp_pubkey(mp_limb_t Q[], const mp_limb_t p[], const mp_limb_t a[], const mp_limb_t G[], const mp_limb_t d[], size_t l) {
	mp_limb_t R[3][l];
	ecp_proj(Q, ecp_mul(*R, d, G, a, p, l), p, l);
}

void ecp_sign(mp_limb_t r[], mp_limb_t s[], const mp_limb_t p[], const mp_limb_t a[], const mp_limb_t G[], const mp_limb_t n[], const mp_limb_t d[], const mp_limb_t z[], size_t l) {
	std::random_device random;
	for (;;) {
		mp_limb_t k[l];
		for (size_t i = 0; i < l; ++i) {
#if GMP_LIMB_BITS == 64
			k[i] = random() | static_cast<mp_limb_t>(random()) << 32;
#elif GMP_LIMB_BITS == 32
			k[i] = random();
#endif
		}
		mp_limb_t R[3][l], S[3][l];
		ecp_proj(*R, ecp_mul(*S, k, G, a, p, l), p, l);
		if (mpn_cmp(R[0], n, l) >= 0) {
			mpn_sub_n(R[0], R[0], n, l);
		}
		if (!mpn_zero_p(R[0], l)) {
			mp_limb_t t0[l], t1[l];
			fp_mul(s, fp_inv(t0, k, n, l), fp_add(t1, z, fp_mul(t1, R[0], d, n, l), n, l), n, l);
			if (!mpn_zero_p(s, l)) {
				mpn_copyi(r, R[0], l);
				break;
			}
		}
	}
}

bool ecp_verify(const mp_limb_t p[], const mp_limb_t a[], const mp_limb_t G[], const mp_limb_t n[], const mp_limb_t Q[], const mp_limb_t z[], const mp_limb_t r[], const mp_limb_t s[], size_t l) {
	mp_limb_t w[l];
	fp_inv(w, s, n, l);
	mp_limb_t u1[l], u2[l];
	fp_mul(u1, z, w, n, l), fp_mul(u2, r, w, n, l);
	mp_limb_t Rp[3][l], T0[3][l], T1[3][l], T2[3][l];
	ecp_proj(*Rp, ecp_add(*T2, ecp_mul(*T0, u1, G, a, p, l), ecp_mul(*T1, u2, Q, a, p, l), a, p, l), p, l);
	if (mpn_cmp(Rp[0], n, l) >= 0) {
		mpn_sub_n(Rp[0], Rp[0], n, l);
	}
	return mpn_cmp(Rp[0], r, l) == 0;
}
