#include "mpn.h"

#include "compiler.h"


void bytes_to_mpn(mp_limb_t mpn[], const std::byte bytes[], size_t n) noexcept {
	if (n == 0) {
		return;
	}
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
			_fallthrough;
		case 7:
			limb |= static_cast<mp_limb_t>(bytes[n - 7]) << 48;
			_fallthrough;
		case 6:
			limb |= static_cast<mp_limb_t>(bytes[n - 6]) << 40;
			_fallthrough;
		case 5:
			limb |= static_cast<mp_limb_t>(bytes[n - 5]) << 32;
			_fallthrough;
#endif
#if GMP_LIMB_BITS >= 32
		case 4:
			limb |= static_cast<mp_limb_t>(bytes[n - 4]) << 24;
			_fallthrough;
		case 3:
			limb |= static_cast<mp_limb_t>(bytes[n - 3]) << 16;
			_fallthrough;
		case 2:
			limb |= static_cast<mp_limb_t>(bytes[n - 2]) << 8;
			_fallthrough;
		case 1:
			limb |= static_cast<mp_limb_t>(bytes[n - 1]);
#endif
	}
	*mpn = limb;
}

void mpn_to_bytes(std::byte bytes[], const mp_limb_t mpn[], size_t n) noexcept {
	if (n == 0) {
		return;
	}
	while (n > sizeof(mp_limb_t)) {
		mp_limb_t limb = *mpn++;
#if GMP_LIMB_BITS >= 64
		bytes[n - 8] = static_cast<std::byte>(limb >> 56);
		bytes[n - 7] = static_cast<std::byte>(limb >> 48);
		bytes[n - 6] = static_cast<std::byte>(limb >> 40);
		bytes[n - 5] = static_cast<std::byte>(limb >> 32);
#endif
#if GMP_LIMB_BITS >= 32
		bytes[n - 4] = static_cast<std::byte>(limb >> 24);
		bytes[n - 3] = static_cast<std::byte>(limb >> 16);
		bytes[n - 2] = static_cast<std::byte>(limb >> 8);
		bytes[n - 1] = static_cast<std::byte>(limb);
#endif
		n -= sizeof(mp_limb_t);
	}
	mp_limb_t limb = *mpn;
	switch (n) {
#if GMP_LIMB_BITS >= 64
		case 8:
			bytes[n - 8] = static_cast<std::byte>(limb >> 56);
			_fallthrough;
		case 7:
			bytes[n - 7] = static_cast<std::byte>(limb >> 48);
			_fallthrough;
		case 6:
			bytes[n - 6] = static_cast<std::byte>(limb >> 40);
			_fallthrough;
		case 5:
			bytes[n - 5] = static_cast<std::byte>(limb >> 32);
			_fallthrough;
#endif
#if GMP_LIMB_BITS >= 32
		case 4:
			bytes[n - 4] = static_cast<std::byte>(limb >> 24);
			_fallthrough;
		case 3:
			bytes[n - 3] = static_cast<std::byte>(limb >> 16);
			_fallthrough;
		case 2:
			bytes[n - 2] = static_cast<std::byte>(limb >> 8);
			_fallthrough;
		case 1:
			bytes[n - 1] = static_cast<std::byte>(limb);
#endif
	}
}
