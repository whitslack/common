#include "scrypt.h"

#include <bit>
#include <future>

#include "compiler.h"
#include "endian.h"
#include "hmac.h"
#include "memory.h"
#include "pbkdf2.h"
#include "sha.h"


static void salsa20_8_core(void * _restrict out, const void * _restrict in) noexcept {
	auto outw = reinterpret_cast<uint32_t *>(out);
	auto inw = reinterpret_cast<const uint32_t *>(in);
	for (size_t i = 0; i < 16; ++i) {
		outw[i] = as_le(inw[i]);
	}
	for (size_t i = 0; i < 4; ++i) {
		outw[ 4] ^= std::rotl(outw[ 0] + outw[12],  7);	outw[ 9] ^= std::rotl(outw[ 5] + outw[ 1],  7);
		outw[14] ^= std::rotl(outw[10] + outw[ 6],  7);	outw[ 3] ^= std::rotl(outw[15] + outw[11],  7);
		outw[ 8] ^= std::rotl(outw[ 4] + outw[ 0],  9);	outw[13] ^= std::rotl(outw[ 9] + outw[ 5],  9);
		outw[ 2] ^= std::rotl(outw[14] + outw[10],  9);	outw[ 7] ^= std::rotl(outw[ 3] + outw[15],  9);
		outw[12] ^= std::rotl(outw[ 8] + outw[ 4], 13);	outw[ 1] ^= std::rotl(outw[13] + outw[ 9], 13);
		outw[ 6] ^= std::rotl(outw[ 2] + outw[14], 13);	outw[11] ^= std::rotl(outw[ 7] + outw[ 3], 13);
		outw[ 0] ^= std::rotl(outw[12] + outw[ 8], 18);	outw[ 5] ^= std::rotl(outw[ 1] + outw[13], 18);
		outw[10] ^= std::rotl(outw[ 6] + outw[ 2], 18);	outw[15] ^= std::rotl(outw[11] + outw[ 7], 18);
		outw[ 1] ^= std::rotl(outw[ 0] + outw[ 3],  7);	outw[ 6] ^= std::rotl(outw[ 5] + outw[ 4],  7);
		outw[11] ^= std::rotl(outw[10] + outw[ 9],  7);	outw[12] ^= std::rotl(outw[15] + outw[14],  7);
		outw[ 2] ^= std::rotl(outw[ 1] + outw[ 0],  9);	outw[ 7] ^= std::rotl(outw[ 6] + outw[ 5],  9);
		outw[ 8] ^= std::rotl(outw[11] + outw[10],  9);	outw[13] ^= std::rotl(outw[12] + outw[15],  9);
		outw[ 3] ^= std::rotl(outw[ 2] + outw[ 1], 13);	outw[ 4] ^= std::rotl(outw[ 7] + outw[ 6], 13);
		outw[ 9] ^= std::rotl(outw[ 8] + outw[11], 13);	outw[14] ^= std::rotl(outw[13] + outw[12], 13);
		outw[ 0] ^= std::rotl(outw[ 3] + outw[ 2], 18);	outw[ 5] ^= std::rotl(outw[ 4] + outw[ 7], 18);
		outw[10] ^= std::rotl(outw[ 9] + outw[ 8], 18);	outw[15] ^= std::rotl(outw[14] + outw[13], 18);
	}
	for (size_t i = 0; i < 16; ++i) {
		as_le(outw[i]) = outw[i] + as_le(inw[i]);
	}
}

static void scrypt_block_mix(void * _restrict out, const void * _restrict in, size_t r) noexcept {
	auto outb = reinterpret_cast<std::byte (*)[64]>(out);
	auto inb = reinterpret_cast<const std::byte (*)[64]>(in);
	auto p = inb[2 * r - 1];
	for (size_t i = 0; i < r; ++i) {
		std::byte t[64];
		for (size_t j = 0; j < 64; ++j) {
			t[j] = p[j] ^ inb[i * 2][j];
		}
		salsa20_8_core(outb[i], t);
		p = outb[i];
		for (size_t j = 0; j < 64; ++j) {
			t[j] = p[j] ^ inb[i * 2 + 1][j];
		}
		salsa20_8_core(outb[r + i], t);
		p = outb[r + i];
	}
}

static void scrypt_ro_mix(void * _restrict out, const void * _restrict in, size_t r, size_t n) {
	auto outb = reinterpret_cast<std::byte *>(out);
	auto buffer = std::make_unique_for_overwrite<std::byte[]>(n * r * 128);
	auto v = reinterpret_cast<std::byte (*)[r * 128]>(buffer.get());
	std::memcpy(v[0], in, sizeof v[0]);
	for (size_t i = 1; i < n; ++i) {
		scrypt_block_mix(v[i], v[i - 1], r);
	}
	scrypt_block_mix(outb, v[n - 1], r);
	for (size_t i = 0; i < n; ++i) {
		std::byte t[r * 128];
		size_t j = *reinterpret_cast<const le<size_t> *>(outb + r * 128 - 64) & n - 1;
		for (size_t k = 0; k < r * 128; ++k) {
			t[k] = outb[k] ^ v[j][k];
		}
		scrypt_block_mix(outb, t, r);
	}
}

void scrypt(void *out, size_t out_len, const void *pass, size_t pass_len, const void *salt, size_t salt_len, size_t r, size_t n, size_t p) {
	auto buffer = std::make_unique_for_overwrite<std::byte[]>(2 * p * r * 128);
	auto b0 = buffer.get(), b1 = b0 + p * r * 128;
	pbkdf2(&prf<HMAC<SHA256>>, 32, pass, pass_len, salt, salt_len, 1, b0, p * r * 128);
	size_t c;
	if (p > 1 && (c = std::thread::hardware_concurrency()) > 1) {
		std::future<void> futures[c];
		for (size_t i = 0, s = 0; i < p; ++i) {
			if (futures[s].valid()) {
				futures[s].get();
			}
			futures[s++] = std::async(std::launch::async, scrypt_ro_mix, b1 + i * r * 128, b0 + i * r * 128, r, n);
			if (s == c) {
				s = 0;
			}
		}
		for (auto &future : futures) {
			if (future.valid()) {
				future.wait();
			}
		}
	}
	else {
		for (size_t i = 0; i < p; ++i) {
			scrypt_ro_mix(b1 + i * r * 128, b0 + i * r * 128, r, n);
		}
	}
	pbkdf2(&prf<HMAC<SHA256>>, 32, pass, pass_len, b1, p * r * 128, 1, out, out_len);
}
