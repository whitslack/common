#include "fp.h"

mp_limb_t * fp_add(mp_limb_t r[], const mp_limb_t n1[], const mp_limb_t n2[], const mp_limb_t p[], size_t l) {
	if (mpn_add_n(r, n1, n2, l) || mpn_cmp(r, p, l) >= 0) {
		mpn_sub_n(r, r, p, l);
	}
	return r;
}

mp_limb_t * fp_sub(mp_limb_t r[], const mp_limb_t n1[], const mp_limb_t n2[], const mp_limb_t p[], size_t l) {
	if (mpn_sub_n(r, n1, n2, l)) {
		mpn_add_n(r, r, p, l);
	}
	return r;
}

mp_limb_t * fp_dbl(mp_limb_t r[], const mp_limb_t n[], const mp_limb_t p[], size_t l) {
	if (mpn_lshift(r, n, l, 1) || mpn_cmp(r, p, l) >= 0) {
		mpn_sub_n(r, r, p, l);
	}
	return r;
}

mp_limb_t * fp_mul(mp_limb_t r[], const mp_limb_t n1[], const mp_limb_t n2[], const mp_limb_t p[], size_t l) {
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

mp_limb_t * fp_mul_1(mp_limb_t r[], const mp_limb_t n1[], mp_limb_t n2, const mp_limb_t p[], size_t l) {
	mpn_zero(r, l);
	bool active = false;
	for (size_t j = sizeof(mp_limb_t) * 8; j > 0; --j) {
		if (active) {
			fp_dbl(r, r, p, l);
		}
		if (static_cast<mp_limb_signed_t>(n2) < 0) {
			fp_add(r, r, n1, p, l);
			active = true;
		}
		n2 <<= 1;
	}
	return r;
}

mp_limb_t * fp_sqr(mp_limb_t r[], const mp_limb_t n[], const mp_limb_t p[], size_t l) {
	return fp_mul(r, n, n, p, l);
}

mp_limb_t * fp_inv(mp_limb_t r[], const mp_limb_t n[], const mp_limb_t p[], size_t l) {
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
