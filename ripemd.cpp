#include "ripemd.h"

#include <bit>

#include "hash.tcc"


static const uint32_t ripemd160_init[5] = { 0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476, 0xc3d2e1f0 };

RIPEMD160::RIPEMD160() noexcept : Hash(ripemd160_init) {
}

void RIPEMD160::update(const uint8_t (&block)[64]) {
	const uint32_t (&words)[16] = reinterpret_cast<const uint32_t (&)[16]>(block);
	uint32_t a = state[0], b = state[1], c = state[2], d = state[3], e = state[4];
#define _(A, B, C, D, E, r, s) A = std::rotl(A + (B ^ C ^ D) + words[r] + 0x00000000, s) + E, C = std::rotl(C, 10)
	_(a, b, c, d, e,  0, 11); _(e, a, b, c, d,  1, 14); _(d, e, a, b, c,  2, 15); _(c, d, e, a, b,  3, 12);
	_(b, c, d, e, a,  4,  5); _(a, b, c, d, e,  5,  8); _(e, a, b, c, d,  6,  7); _(d, e, a, b, c,  7,  9);
	_(c, d, e, a, b,  8, 11); _(b, c, d, e, a,  9, 13); _(a, b, c, d, e, 10, 14); _(e, a, b, c, d, 11, 15);
	_(d, e, a, b, c, 12,  6); _(c, d, e, a, b, 13,  7); _(b, c, d, e, a, 14,  9); _(a, b, c, d, e, 15,  8);
#undef _
#define _(A, B, C, D, E, r, s) A = std::rotl(A + (B & C | ~B & D) + words[r] + 0x5a827999, s) + E, C = std::rotl(C, 10)
	_(e, a, b, c, d,  7,  7); _(d, e, a, b, c,  4,  6); _(c, d, e, a, b, 13,  8); _(b, c, d, e, a,  1, 13);
	_(a, b, c, d, e, 10, 11); _(e, a, b, c, d,  6,  9); _(d, e, a, b, c, 15,  7); _(c, d, e, a, b,  3, 15);
	_(b, c, d, e, a, 12,  7); _(a, b, c, d, e,  0, 12); _(e, a, b, c, d,  9, 15); _(d, e, a, b, c,  5,  9);
	_(c, d, e, a, b,  2, 11); _(b, c, d, e, a, 14,  7); _(a, b, c, d, e, 11, 13); _(e, a, b, c, d,  8, 12);
#undef _
#define _(A, B, C, D, E, r, s) A = std::rotl(A + ((B | ~C) ^ D) + words[r] + 0x6ed9eba1, s) + E, C = std::rotl(C, 10)
	_(d, e, a, b, c,  3, 11); _(c, d, e, a, b, 10, 13); _(b, c, d, e, a, 14,  6); _(a, b, c, d, e,  4,  7);
	_(e, a, b, c, d,  9, 14); _(d, e, a, b, c, 15,  9); _(c, d, e, a, b,  8, 13); _(b, c, d, e, a,  1, 15);
	_(a, b, c, d, e,  2, 14); _(e, a, b, c, d,  7,  8); _(d, e, a, b, c,  0, 13); _(c, d, e, a, b,  6,  6);
	_(b, c, d, e, a, 13,  5); _(a, b, c, d, e, 11, 12); _(e, a, b, c, d,  5,  7); _(d, e, a, b, c, 12,  5);
#undef _
#define _(A, B, C, D, E, r, s) A = std::rotl(A + (B & D | C & ~D) + words[r] + 0x8f1bbcdc, s) + E, C = std::rotl(C, 10)
	_(c, d, e, a, b,  1, 11); _(b, c, d, e, a,  9, 12); _(a, b, c, d, e, 11, 14); _(e, a, b, c, d, 10, 15);
	_(d, e, a, b, c,  0, 14); _(c, d, e, a, b,  8, 15); _(b, c, d, e, a, 12,  9); _(a, b, c, d, e,  4,  8);
	_(e, a, b, c, d, 13,  9); _(d, e, a, b, c,  3, 14); _(c, d, e, a, b,  7,  5); _(b, c, d, e, a, 15,  6);
	_(a, b, c, d, e, 14,  8); _(e, a, b, c, d,  5,  6); _(d, e, a, b, c,  6,  5); _(c, d, e, a, b,  2, 12);
#undef _
#define _(A, B, C, D, E, r, s) A = std::rotl(A + (B ^ (C | ~D)) + words[r] + 0xa953fd4e, s) + E, C = std::rotl(C, 10)
	_(b, c, d, e, a,  4,  9); _(a, b, c, d, e,  0, 15); _(e, a, b, c, d,  5,  5); _(d, e, a, b, c,  9, 11);
	_(c, d, e, a, b,  7,  6); _(b, c, d, e, a, 12,  8); _(a, b, c, d, e,  2, 13); _(e, a, b, c, d, 10, 12);
	_(d, e, a, b, c, 14,  5); _(c, d, e, a, b,  1, 12); _(b, c, d, e, a,  3, 13); _(a, b, c, d, e,  8, 14);
	_(e, a, b, c, d, 11, 11); _(d, e, a, b, c,  6,  8); _(c, d, e, a, b, 15,  5); _(b, c, d, e, a, 13,  6);
#undef _
	uint32_t t = a;
	state[0] = (a = state[0]) + b, state[1] = (b = state[1]) + c, state[2] = (c = state[2]) + d, state[3] = (d = state[3]) + e, state[4] = (e = state[4]) + t;
#define _(A, B, C, D, E, r, s) A = std::rotl(A + (B ^ (C | ~D)) + words[r] + 0x50a28be6, s) + E, C = std::rotl(C, 10)
	_(a, b, c, d, e,  5,  8); _(e, a, b, c, d, 14,  9); _(d, e, a, b, c,  7,  9); _(c, d, e, a, b,  0, 11);
	_(b, c, d, e, a,  9, 13); _(a, b, c, d, e,  2, 15); _(e, a, b, c, d, 11, 15); _(d, e, a, b, c,  4,  5);
	_(c, d, e, a, b, 13,  7); _(b, c, d, e, a,  6,  7); _(a, b, c, d, e, 15,  8); _(e, a, b, c, d,  8, 11);
	_(d, e, a, b, c,  1, 14); _(c, d, e, a, b, 10, 14); _(b, c, d, e, a,  3, 12); _(a, b, c, d, e, 12,  6);
#undef _
#define _(A, B, C, D, E, r, s) A = std::rotl(A + (B & D | C & ~D) + words[r] + 0x5c4dd124, s) + E, C = std::rotl(C, 10)
	_(e, a, b, c, d,  6,  9); _(d, e, a, b, c, 11, 13); _(c, d, e, a, b,  3, 15); _(b, c, d, e, a,  7,  7);
	_(a, b, c, d, e,  0, 12); _(e, a, b, c, d, 13,  8); _(d, e, a, b, c,  5,  9); _(c, d, e, a, b, 10, 11);
	_(b, c, d, e, a, 14,  7); _(a, b, c, d, e, 15,  7); _(e, a, b, c, d,  8, 12); _(d, e, a, b, c, 12,  7);
	_(c, d, e, a, b,  4,  6); _(b, c, d, e, a,  9, 15); _(a, b, c, d, e,  1, 13); _(e, a, b, c, d,  2, 11);
#undef _
#define _(A, B, C, D, E, r, s) A = std::rotl(A + ((B | ~C) ^ D) + words[r] + 0x6d703ef3, s) + E, C = std::rotl(C, 10)
	_(d, e, a, b, c, 15,  9); _(c, d, e, a, b,  5,  7); _(b, c, d, e, a,  1, 15); _(a, b, c, d, e,  3, 11);
	_(e, a, b, c, d,  7,  8); _(d, e, a, b, c, 14,  6); _(c, d, e, a, b,  6,  6); _(b, c, d, e, a,  9, 14);
	_(a, b, c, d, e, 11, 12); _(e, a, b, c, d,  8, 13); _(d, e, a, b, c, 12,  5); _(c, d, e, a, b,  2, 14);
	_(b, c, d, e, a, 10, 13); _(a, b, c, d, e,  0, 13); _(e, a, b, c, d,  4,  7); _(d, e, a, b, c, 13,  5);
#undef _
#define _(A, B, C, D, E, r, s) A = std::rotl(A + (B & C | ~B & D) + words[r] + 0x7a6d76e9, s) + E, C = std::rotl(C, 10)
	_(c, d, e, a, b,  8, 15); _(b, c, d, e, a,  6,  5); _(a, b, c, d, e,  4,  8); _(e, a, b, c, d,  1, 11);
	_(d, e, a, b, c,  3, 14); _(c, d, e, a, b, 11, 14); _(b, c, d, e, a, 15,  6); _(a, b, c, d, e,  0, 14);
	_(e, a, b, c, d,  5,  6); _(d, e, a, b, c, 12,  9); _(c, d, e, a, b,  2, 12); _(b, c, d, e, a, 13,  9);
	_(a, b, c, d, e,  9, 12); _(e, a, b, c, d,  7,  5); _(d, e, a, b, c, 10, 15); _(c, d, e, a, b, 14,  8);
#undef _
#define _(A, B, C, D, E, r, s) A = std::rotl(A + (B ^ C ^ D) + words[r] + 0x00000000, s) + E, C = std::rotl(C, 10)
	_(b, c, d, e, a, 12,  8); _(a, b, c, d, e, 15,  5); _(e, a, b, c, d, 10, 12); _(d, e, a, b, c,  4,  9);
	_(c, d, e, a, b,  1, 12); _(b, c, d, e, a,  5,  5); _(a, b, c, d, e,  8, 14); _(e, a, b, c, d,  7,  6);
	_(d, e, a, b, c,  6,  8); _(c, d, e, a, b,  2, 13); _(b, c, d, e, a, 13,  6); _(a, b, c, d, e, 14,  5);
	_(e, a, b, c, d,  0, 15); _(d, e, a, b, c,  3, 13); _(c, d, e, a, b,  9, 11); _(b, c, d, e, a, 11, 11);
#undef _
	t = state[0], state[0] = state[1] + d, state[1] = state[2] + e, state[2] = state[3] + a, state[3] = state[4] + b, state[4] = t + c;
}


template class Hash<64, 20, 20, uint32_t, uint64_t, false>;


#include "hmac.tcc"

template class HMAC<RIPEMD160>;
