#pragma once

#include <array>

#include "compiler.h"
#include "pp.h"


#if SIZE_MAX >= UINT64_MAX
#	define WORDS_C(v) (UINT64_C(v))
#elif SIZE_MAX >= UINT32_MAX
#	if BYTE_ORDER == BIG_ENDIAN
#		define WORDS_C(v) static_cast<uint32_t>((UINT64_C(v)) >> 32), static_cast<uint32_t>(UINT64_C(v))
#	elif BYTE_ORDER == LITTLE_ENDIAN
#		define WORDS_C(v) static_cast<uint32_t>(UINT64_C(v)), static_cast<uint32_t>((UINT64_C(v)) >> 32)
#	endif
#elif SIZE_MAX >= UINT16_MAX
#	if BYTE_ORDER == BIG_ENDIAN
#		define WORDS_C(v) static_cast<uint16_t>((UINT64_C(v)) >> 48), static_cast<uint16_t>((UINT64_C(v)) >> 32), static_cast<uint16_t>((UINT64_C(v)) >> 16), static_cast<uint16_t>(UINT64_C(v))
#	elif BYTE_ORDER == LITTLE_ENDIAN
#		define WORDS_C(v) static_cast<uint16_t>(UINT64_C(v)), static_cast<uint16_t>((UINT64_C(v)) >> 16), static_cast<uint16_t>((UINT64_C(v)) >> 32), static_cast<uint16_t>((UINT64_C(v)) >> 48)
#	endif
#else
#	error "unsupported CPU word size"
#endif

#if BYTE_ORDER == BIG_ENDIAN
#	define WORD(L, i) ((L) - 1 - (i))
#	define BIGUINT_INIT(seq) { LIST(MAP(seq, WORDS_C)) }
#elif BYTE_ORDER == LITTLE_ENDIAN
#	define WORD(L, i) (i)
#	define BIGUINT_INIT(seq) { LIST(MAP(REVERSE(seq), WORDS_C)) }
#else
#	error "unsupported CPU byte order"
#endif


template <typename LIMB_T, size_t L>
static inline std::enable_if_t<sizeof(LIMB_T) < sizeof(uintptr_t), void> __add(std::array<LIMB_T, L> &sum, const std::array<LIMB_T, L> &augend, LIMB_T addend) noexcept {
	intptr_t carry = addend;
	for (size_t i = 0; i < L; ++i) {
		sum[WORD(L, i)] = static_cast<LIMB_T>(carry += augend[WORD(L, i)]), carry >>= sizeof(LIMB_T) * 8;
	}
}

template <typename LIMB_T, size_t L>
static inline std::enable_if_t<sizeof(LIMB_T) < sizeof(uintptr_t), void> __add(std::array<LIMB_T, L> &sum, const std::array<LIMB_T, L> &augend, const std::array<LIMB_T, L> &addend) noexcept {
	intptr_t carry = 0;
	for (size_t i = 0; i < L; ++i) {
		sum[WORD(L, i)] = static_cast<LIMB_T>((carry += augend[WORD(L, i)]) += addend[WORD(L, i)]), carry >>= sizeof(LIMB_T) * 8;
	}
}

template <typename LIMB_T, size_t L>
static inline std::array<LIMB_T, L> & operator += (std::array<LIMB_T, L> &augend, LIMB_T addend) noexcept {
	if (addend != 0) {
		__add(augend, augend, addend);
	}
	return augend;
}

template <typename LIMB_T, size_t L>
static inline std::array<LIMB_T, L> & operator += (std::array<LIMB_T, L> &augend, const std::array<LIMB_T, L> &addend) noexcept {
	__add(augend, augend, addend);
	return augend;
}

template <typename LIMB_T, size_t L>
static inline std::array<LIMB_T, L> _pure operator + (const std::array<LIMB_T, L> &augend, LIMB_T addend) noexcept {
	std::array<LIMB_T, L> sum;
	__add(sum, augend, addend);
	return sum;
}

template <typename LIMB_T, size_t L>
static inline std::array<LIMB_T, L> _pure operator + (const std::array<LIMB_T, L> &augend, const std::array<LIMB_T, L> &addend) noexcept {
	std::array<LIMB_T, L> sum;
	__add(sum, augend, addend);
	return sum;
}

template <typename LIMB_T, size_t L>
static inline std::enable_if_t<sizeof(LIMB_T) < sizeof(uintptr_t), void> __sub(std::array<LIMB_T, L> &difference, const std::array<LIMB_T, L> &minuend, LIMB_T subtrahend) noexcept {
	intptr_t borrow = -subtrahend;
	for (size_t i = 0; i < L; ++i) {
		difference[WORD(L, i)] = static_cast<LIMB_T>(borrow += minuend[WORD(L, i)]), borrow >>= sizeof(LIMB_T) * 8;
	}
}

template <typename LIMB_T, size_t L>
static inline std::enable_if_t<sizeof(LIMB_T) < sizeof(uintptr_t), void> __sub(std::array<LIMB_T, L> &difference, const std::array<LIMB_T, L> &minuend, const std::array<LIMB_T, L> &subtrahend) noexcept {
	intptr_t borrow = 0;
	for (size_t i = 0; i < L; ++i) {
		difference[WORD(L, i)] = static_cast<LIMB_T>((borrow += minuend[WORD(L, i)]) -= subtrahend[WORD(L, i)]), borrow >>= sizeof(LIMB_T) * 8;
	}
}

template <typename LIMB_T, size_t L>
static inline std::array<LIMB_T, L> & operator -= (std::array<LIMB_T, L> &minuend, LIMB_T subtrahend) noexcept {
	if (subtrahend != 0) {
		__sub(minuend, minuend, subtrahend);
	}
	return minuend;
}

template <typename LIMB_T, size_t L>
static inline std::array<LIMB_T, L> & operator -= (std::array<LIMB_T, L> &minuend, const std::array<LIMB_T, L> &subtrahend) noexcept {
	__sub(minuend, minuend, subtrahend);
	return minuend;
}

template <typename LIMB_T, size_t L>
static inline std::array<LIMB_T, L> _pure operator - (const std::array<LIMB_T, L> &minuend, LIMB_T subtrahend) noexcept {
	std::array<LIMB_T, L> difference;
	__sub(difference, minuend, subtrahend);
	return difference;
}

template <typename LIMB_T, size_t L>
static inline std::array<LIMB_T, L> _pure operator - (const std::array<LIMB_T, L> &minuend, const std::array<LIMB_T, L> &subtrahend) noexcept {
	std::array<LIMB_T, L> difference;
	__sub(difference, minuend, subtrahend);
	return difference;
}

template <typename WORD_T, size_t W>
static inline std::enable_if_t<std::is_unsigned_v<WORD_T>, void> __shln(std::array<WORD_T, W> &result, const std::array<WORD_T, W> &operand, unsigned shift) noexcept {
	unsigned offset = static_cast<unsigned>(shift / (sizeof(WORD_T) * 8));
	if ((shift %= static_cast<unsigned>(sizeof(WORD_T) * 8)) == 0) {
		for (size_t i = W - offset; i-- > 0;) {
			result[WORD(W, i + offset)] = operand[WORD(W, i)];
		}
	}
	else {
		for (size_t i = W - offset; i-- > 1;) {
			result[WORD(W, i + offset)] = static_cast<WORD_T>(operand[WORD(W, i)] << shift | operand[WORD(W, i - 1)] >> sizeof(WORD_T) * 8 - shift);
		}
		result[WORD(W, offset)] = static_cast<WORD_T>(operand[WORD(W, 0)] << shift);
	}
	for (size_t i = offset; i-- > 0;) {
		result[WORD(W, i)] = 0;
	}
}

template <typename WORD_T, size_t W>
static inline std::array<WORD_T, W> & operator <<= (std::array<WORD_T, W> &operand, unsigned shift) noexcept {
	if (shift != 0) {
		__shln(operand, operand, shift);
	}
	return operand;
}

template <typename WORD_T, size_t W>
static inline std::array<WORD_T, W> _pure operator << (const std::array<WORD_T, W> &operand, unsigned shift) noexcept {
	std::array<WORD_T, W> result;
	__shln(result, operand, shift);
	return result;
}

template <typename WORD_T, size_t W>
static inline std::enable_if_t<std::is_unsigned_v<WORD_T>, void> __shrn(std::array<WORD_T, W> &result, const std::array<WORD_T, W> &operand, unsigned shift) noexcept {
	unsigned offset = static_cast<unsigned>(shift / (sizeof(WORD_T) * 8));
	if ((shift %= static_cast<unsigned>(sizeof(WORD_T) * 8)) == 0) {
		for (size_t i = 0; i < W - offset; ++i) {
			result[WORD(W, i)] = operand[WORD(W, i + offset)];
		}
	}
	else {
		for (size_t i = 0; i < W - offset - 1; ++i) {
			result[WORD(W, i)] = static_cast<WORD_T>(operand[WORD(W, i + offset)] >> shift | operand[WORD(W, i + offset + 1)] << sizeof(WORD_T) * 8 - shift);
		}
		result[WORD(W, W - offset - 1)] = static_cast<WORD_T>(operand[WORD(W, W - 1)] >> shift);
	}
	for (size_t i = offset; i > 0;) {
		result[WORD(W, W - i--)] = 0;
	}
}

template <typename WORD_T, size_t W>
static inline std::array<WORD_T, W> & operator >>= (std::array<WORD_T, W> &operand, unsigned shift) noexcept {
	if (shift != 0) {
		__shrn(operand, operand, shift);
	}
	return operand;
}

template <typename WORD_T, size_t W>
static inline std::array<WORD_T, W> _pure operator >> (const std::array<WORD_T, W> &operand, unsigned shift) noexcept {
	std::array<WORD_T, W> result;
	__shrn(result, operand, shift);
	return result;
}

template <typename WORD_T, size_t W>
static inline void __com(std::array<WORD_T, W> &result, const std::array<WORD_T, W> &operand) noexcept {
	for (size_t i = 0; i < W; ++i) {
		result[WORD(W, i)] = ~operand[WORD(W, i)];
	}
}

template <typename WORD_T, size_t W>
static inline std::array<WORD_T, W> _pure operator ~ (const std::array<WORD_T, W> &operand) noexcept {
	std::array<WORD_T, W> result;
	__com(result, operand);
	return result;
}

template <typename WORD_T, size_t W>
static inline void __and(std::array<WORD_T, W> &result, const std::array<WORD_T, W> &operand1, const std::array<WORD_T, W> &operand2) noexcept {
	for (size_t i = 0; i < W; ++i) {
		result[WORD(W, i)] = operand1[WORD(W, i)] & operand2[WORD(W, i)];
	}
}

template <typename WORD_T, size_t W>
static inline std::array<WORD_T, W> & operator &= (std::array<WORD_T, W> &operand1, const std::array<WORD_T, W> &operand2) noexcept {
	__and(operand1, operand1, operand2);
	return operand1;
}

template <typename WORD_T, size_t W>
static inline std::array<WORD_T, W> _pure operator & (const std::array<WORD_T, W> &operand1, const std::array<WORD_T, W> &operand2) noexcept {
	std::array<WORD_T, W> result;
	__and(result, operand1, operand2);
	return result;
}

template <typename WORD_T, size_t W>
static inline void __or(std::array<WORD_T, W> &result, const std::array<WORD_T, W> &operand1, const std::array<WORD_T, W> &operand2) noexcept {
	for (size_t i = 0; i < W; ++i) {
		result[WORD(W, i)] = operand1[WORD(W, i)] | operand2[WORD(W, i)];
	}
}

template <typename WORD_T, size_t W>
static inline std::array<WORD_T, W> & operator |= (std::array<WORD_T, W> &operand1, const std::array<WORD_T, W> &operand2) noexcept {
	__or(operand1, operand1, operand2);
	return operand1;
}

template <typename WORD_T, size_t W>
static inline std::array<WORD_T, W> _pure operator | (const std::array<WORD_T, W> &operand1, const std::array<WORD_T, W> &operand2) noexcept {
	std::array<WORD_T, W> result;
	__or(result, operand1, operand2);
	return result;
}

template <typename WORD_T, size_t W>
static inline void __xor(std::array<WORD_T, W> &result, const std::array<WORD_T, W> &operand1, const std::array<WORD_T, W> &operand2) noexcept {
	for (size_t i = 0; i < W; ++i) {
		result[WORD(W, i)] = operand1[WORD(W, i)] ^ operand2[WORD(W, i)];
	}
}

template <typename WORD_T, size_t W>
static inline std::array<WORD_T, W> & operator ^= (std::array<WORD_T, W> &operand1, const std::array<WORD_T, W> &operand2) noexcept {
	__xor(operand1, operand1, operand2);
	return operand1;
}

template <typename WORD_T, size_t W>
static inline std::array<WORD_T, W> _pure operator ^ (const std::array<WORD_T, W> &operand1, const std::array<WORD_T, W> &operand2) noexcept {
	std::array<WORD_T, W> result;
	__xor(result, operand1, operand2);
	return result;
}

template <typename WORD_T, size_t W>
static inline int _pure __cmp(const std::array<WORD_T, W> &lhs, const std::array<WORD_T, W> &rhs) noexcept {
	for (size_t i = W; i-- > 0;) {
		auto lw = lhs[WORD(W, i)], rw = rhs[WORD(W, i)];
		if (lw < rw) {
			return -1;
		}
		if (lw > rw) {
			return 1;
		}
	}
	return 0;
}

template <typename WORD_T, size_t W>
static inline size_t _pure __clz(const std::array<WORD_T, W> &operand) noexcept {
	size_t ret = 0;
	for (size_t i = W; i > 0;) {
		auto word = operand[WORD(W, --i)];
		if (word) {
			ret += _clz(word);
			break;
		}
		ret += sizeof word * 8;
	}
	return ret;
}

template <typename WORD_T, size_t W>
static inline size_t _pure __ctz(const std::array<WORD_T, W> &operand) noexcept {
	size_t ret = 0;
	for (size_t i = 0; i < W; ++i) {
		auto word = operand[WORD(W, i)];
		if (word) {
			ret += _ctz(word);
			break;
		}
		ret += sizeof word * 8;
	}
	return ret;
}

template <typename WORD_T, size_t W>
static inline size_t _pure __popcount(const std::array<WORD_T, W> &operand) noexcept {
	size_t ret = 0;
	for (size_t i = 0; i < W; ++i) {
		ret += _popcount(operand[WORD(W, i)]);
	}
	return ret;
}

template <typename WORD_T, size_t W>
static inline size_t _pure __parity(const std::array<WORD_T, W> &operand) noexcept {
	size_t ret = 0;
	for (size_t i = 0; i < W; ++i) {
		ret ^= _parity(operand[WORD(W, i)]);
	}
	return ret;
}

template <typename WORD_T, size_t W>
static inline void __bswap(std::array<WORD_T, W> &operand) noexcept {
	for (size_t i = 0; i < W / 2; ++i) {
		auto temp = operand[i];
		operand[i] = bswap(operand[W - i - 1]);
		operand[W - i - 1] = bswap(temp);
	}
	if (W / 2 != (W + 1) / 2) {
		operand[W / 2] = bswap(operand[W / 2]);
	}
}

#if defined(__GNUC__) && (defined(__amd64__) || defined(__i386__))

#define __IMPL(WORD_T, SUFFIX) \
	\
	template <size_t L> \
	static inline void __adc(WORD_T augend[]) noexcept { \
		__asm__ ("adc" SUFFIX " $0, %[augend]" : [augend] "+rm" (augend[0]) : "X" (augend[-1]) : "cc"); \
		__adc<L - 1>(augend + 1); \
	} \
	\
	template <> \
	inline void __adc<0>(WORD_T *) noexcept { \
	} \
	\
	template <size_t L> \
	static inline void __adc(WORD_T augend[], const WORD_T addend[]) noexcept { \
		__asm__ ("adc" SUFFIX " %[addend], %[augend]" : [augend] "+r,m" (augend[0]) : [addend] "irm,ir" (addend[0]), "X,X" (augend[-1]) : "cc"); \
		__adc<L - 1>(augend + 1, addend + 1); \
	} \
	\
	template <> \
	inline void __adc<0>(WORD_T *, const WORD_T *) noexcept { \
	} \
	\
	template <size_t L> \
	static inline void __add(WORD_T augend[], WORD_T addend) noexcept { \
		__asm__ ("add" SUFFIX " %[addend], %[augend]" : [augend] "+r,m" (augend[0]) : [addend] "irm,ir" (addend) : "cc"); \
		__adc<L - 1>(augend + 1); \
	} \
	\
	template <size_t L> \
	static inline void __add(WORD_T augend[], const WORD_T addend[]) noexcept { \
		__asm__ ("add" SUFFIX " %[addend], %[augend]" : [augend] "+r,m" (augend[0]) : [addend] "irm,ir" (addend[0]) : "cc"); \
		__adc<L - 1>(augend + 1, addend + 1); \
	} \
	\
	template <size_t L> \
	static inline std::array<WORD_T, L> & operator += (std::array<WORD_T, L> &augend, WORD_T addend) noexcept { \
		__add<L>(augend.data(), addend); \
		return augend; \
	} \
	\
	template <size_t L> \
	static inline std::array<WORD_T, L> & operator += (std::array<WORD_T, L> &augend, const std::array<WORD_T, L> &addend) noexcept { \
		__add<L>(augend.data(), addend.data()); \
		return augend; \
	} \
	\
	template <size_t L> \
	static inline std::array<WORD_T, L> _pure operator + (std::array<WORD_T, L> augend, WORD_T addend) noexcept { \
		return augend += addend; \
	} \
	\
	template <size_t L> \
	static inline std::array<WORD_T, L> _pure operator + (std::array<WORD_T, L> augend, const std::array<WORD_T, L> &addend) noexcept { \
		return augend += addend; \
	} \
	\
	template <size_t L> \
	static inline void __sbb(WORD_T minuend[]) noexcept { \
		__asm__ ("sbb" SUFFIX " $0, %[minuend]" : [minuend] "+rm" (minuend[0]) : "X" (minuend[-1]) : "cc"); \
		__sbb<L - 1>(minuend + 1); \
	} \
	\
	template <> \
	inline void __sbb<0>(WORD_T *) noexcept { \
	} \
	\
	template <size_t L> \
	static inline void __sbb(WORD_T minuend[], const WORD_T subtrahend[]) noexcept { \
		__asm__ ("sbb" SUFFIX " %[subtrahend], %[minuend]" : [minuend] "+r,m" (minuend[0]) : [subtrahend] "irm,ir" (subtrahend[0]), "X,X" (minuend[-1]) : "cc"); \
		__sbb<L - 1>(minuend + 1, subtrahend + 1); \
	} \
	\
	template <> \
	inline void __sbb<0>(WORD_T *, const WORD_T *) noexcept { \
	} \
	\
	template <size_t L> \
	static inline void __sub(WORD_T minuend[], WORD_T subtrahend) noexcept { \
		__asm__ ("sub" SUFFIX " %[subtrahend], %[minuend]" : [minuend] "+r,m" (minuend[0]) : [subtrahend] "irm,ir" (subtrahend) : "cc"); \
		__sbb<L - 1>(minuend + 1); \
	} \
	\
	template <size_t L> \
	static inline void __sub(WORD_T minuend[], const WORD_T subtrahend[]) noexcept { \
		__asm__ ("sub" SUFFIX " %[subtrahend], %[minuend]" : [minuend] "+r,m" (minuend[0]) : [subtrahend] "irm,ir" (subtrahend[0]) : "cc"); \
		__sbb<L - 1>(minuend + 1, subtrahend + 1); \
	} \
	\
	template <size_t L> \
	static inline std::array<WORD_T, L> & operator -= (std::array<WORD_T, L> &minuend, WORD_T subtrahend) noexcept { \
		__sub<L>(minuend.data(), subtrahend); \
		return minuend; \
	} \
	\
	template <size_t L> \
	static inline std::array<WORD_T, L> & operator -= (std::array<WORD_T, L> &minuend, const std::array<WORD_T, L> &subtrahend) noexcept { \
		__sub<L>(minuend.data(), subtrahend.data()); \
		return minuend; \
	} \
	\
	template <size_t L> \
	static inline std::array<WORD_T, L> _pure operator - (std::array<WORD_T, L> minuend, WORD_T subtrahend) noexcept { \
		return minuend -= subtrahend; \
	} \
	\
	template <size_t L> \
	static inline std::array<WORD_T, L> _pure operator - (std::array<WORD_T, L> minuend, const std::array<WORD_T, L> &subtrahend) noexcept { \
		return minuend -= subtrahend; \
	} \
	\
	template <size_t L> \
	static inline void __rcl(WORD_T operand[]) noexcept { \
		__asm__ ("rcl" SUFFIX " %[operand]" : [operand] "+rm" (operand[0]) : "X" (operand[-1]) : "cc"); \
		__rcl<L - 1>(operand + 1); \
	} \
	\
	template <> \
	inline void __rcl<0>(WORD_T *) noexcept { \
	} \
	\
	template <size_t L> \
	static inline void __shl(WORD_T operand[]) noexcept { \
		__asm__ ("shl" SUFFIX " %[operand]" : [operand] "+rm" (operand[0]) : : "cc"); \
		__rcl<L - 1>(operand + 1); \
	} \
	\
	template <size_t L> \
	static inline std::array<WORD_T, L> & operator <<= (std::array<WORD_T, L> &operand, unsigned shift) noexcept { \
		if (shift == 1) { \
			__shl<L>(operand.data()); \
		} \
		else if (shift != 0) { \
			__shln(operand, operand, shift); \
		} \
		return operand; \
	} \
	\
	template <size_t L> \
	static inline std::array<WORD_T, L> _pure operator << (const std::array<WORD_T, L> &operand, unsigned shift) noexcept { \
		std::array<WORD_T, L> result; \
		if (shift == 1) { \
			__shl<L>((result = operand).data()); \
		} \
		else { \
			__shln(result, operand, shift); \
		} \
		return result; \
	} \
	\
	template <size_t L> \
	static inline void __rcr(WORD_T operand[]) noexcept { \
		__asm__ ("rcr" SUFFIX " %[operand]" : [operand] "+rm" (operand[L - 1]) : "X" (operand[L]) : "cc"); \
		__rcr<L - 1>(operand); \
	} \
	\
	template <> \
	inline void __rcr<0>(WORD_T *) noexcept { \
	} \
	\
	template <size_t L> \
	static inline void __shr(WORD_T operand[]) noexcept { \
		__asm__ ("shr" SUFFIX " %[operand]" : [operand] "+rm" (operand[L - 1]) : : "cc"); \
		__rcr<L - 1>(operand); \
	} \
	\
	template <size_t L> \
	static inline std::array<WORD_T, L> & operator >>= (std::array<WORD_T, L> &operand, unsigned shift) noexcept { \
		if (shift == 1) { \
			__shr<L>(operand.data()); \
		} \
		else if (shift != 0) { \
			__shrn(operand, operand, shift); \
		} \
		return operand; \
	} \
	\
	template <size_t L> \
	static inline std::array<WORD_T, L> _pure operator >> (const std::array<WORD_T, L> &operand, unsigned shift) noexcept { \
		std::array<WORD_T, L> result; \
		if (shift == 1) { \
			__shr<L>((result = operand).data()); \
		} \
		else { \
			__shrn(result, operand, shift); \
		} \
		return result; \
	}

#ifdef __amd64__
#define LIMB_T uint64_t
__IMPL(uint64_t, "q")
#else
#define LIMB_T uint32_t
#endif
__IMPL(uint32_t, "l")
#undef __IMPL

#elif defined(__GNUC__) && defined(__arm__)

#define LIMB_T uint32_t

template <size_t L>
static inline void __adc(uint32_t sum[], const uint32_t augend[]) noexcept {
	__asm__ ("adcs %[sum], %[augend], #0" : [sum] "=r" (sum[0]) : [augend] "r" (augend[0]), "X" (sum[-1]) : "cc");
	__adc<L - 1>(sum + 1, augend + 1);
}

template <>
inline void __adc<0>(uint32_t *, const uint32_t *) noexcept {
}

template <size_t L>
static inline void __adc(uint32_t sum[], const uint32_t augend[], const uint32_t addend[]) noexcept {
	__asm__ ("adcs %[sum], %[augend], %[addend]" : [sum] "=r" (sum[0]) : [augend] "r" (augend[0]), [addend] "Ir" (addend[0]), "X" (sum[-1]) : "cc");
	__adc<L - 1>(sum + 1, augend + 1, addend + 1);
}

template <>
inline void __adc<0>(uint32_t *, const uint32_t *, const uint32_t *) noexcept {
}

template <size_t L>
static inline void __add(uint32_t sum[], const uint32_t augend[], uint32_t addend) noexcept {
	__asm__ ("adds %[sum], %[augend], %[addend]" : [sum] "=r" (sum[0]) : [augend] "r" (augend[0]), [addend] "Ir" (addend) : "cc");
	__adc<L - 1>(sum + 1, augend + 1);
}

template <size_t L>
static inline void __add(uint32_t sum[], const uint32_t augend[], const uint32_t addend[]) noexcept {
	__asm__ ("adds %[sum], %[augend], %[addend]" : [sum] "=r" (sum[0]) : [augend] "r" (augend[0]), [addend] "Ir" (addend[0]) : "cc");
	__adc<L - 1>(sum + 1, augend + 1, addend + 1);
}

template <size_t L>
static inline std::array<uint32_t, L> & operator += (std::array<uint32_t, L> &augend, uint32_t addend) noexcept {
	__add<L>(augend.data(), augend.data(), addend);
	return augend;
}

template <size_t L>
static inline std::array<uint32_t, L> & operator += (std::array<uint32_t, L> &augend, const std::array<uint32_t, L> &addend) noexcept {
	__add<L>(augend.data(), augend.data(), addend.data());
	return augend;
}

template <size_t L>
static inline std::array<uint32_t, L> _pure operator + (const std::array<uint32_t, L> &augend, uint32_t addend) noexcept {
	std::array<uint32_t, L> sum;
	__add<L>(sum.data(), augend.data(), addend);
	return sum;
}

template <size_t L>
static inline std::array<uint32_t, L> _pure operator + (const std::array<uint32_t, L> &augend, const std::array<uint32_t, L> &addend) noexcept {
	std::array<uint32_t, L> sum;
	__add<L>(sum.data(), augend.data(), addend.data());
	return sum;
}

template <size_t L>
static inline void __sbc(uint32_t difference[], const uint32_t minuend[]) noexcept {
	__asm__ ("sbcs %[difference], %[minuend], #0" : [difference] "=r" (difference[0]) : [minuend] "r" (minuend[0]), "X" (difference[-1]) : "cc");
	__sbc<L - 1>(difference + 1, minuend + 1);
}

template <>
inline void __sbc<0>(uint32_t *, const uint32_t *) noexcept {
}

template <size_t L>
static inline void __sbc(uint32_t difference[], const uint32_t minuend[], const uint32_t subtrahend[]) noexcept {
	__asm__ ("sbcs %[difference], %[minuend], %[subtrahend]" : [difference] "=r" (difference[0]) : [minuend] "r" (minuend[0]), [subtrahend] "Ir" (subtrahend[0]), "X" (difference[-1]) : "cc");
	__sbc<L - 1>(difference + 1, minuend + 1, subtrahend + 1);
}

template <>
inline void __sbc<0>(uint32_t *, const uint32_t *, const uint32_t *) noexcept {
}

template <size_t L>
static inline void __sub(uint32_t difference[], const uint32_t minuend[], uint32_t subtrahend) noexcept {
	__asm__ ("subs %[difference], %[minuend], %[subtrahend]" : [difference] "=r" (difference[0]) : [minuend] "r" (minuend[0]), [subtrahend] "Ir" (subtrahend) : "cc");
	__sbc<L - 1>(difference + 1, minuend + 1);
}

template <size_t L>
static inline void __sub(uint32_t difference[], const uint32_t minuend[], const uint32_t subtrahend[]) noexcept {
	__asm__ ("subs %[difference], %[minuend], %[subtrahend]" : [difference] "=r" (difference[0]) : [minuend] "r" (minuend[0]), [subtrahend] "Ir" (subtrahend[0]) : "cc");
	__sbc<L - 1>(difference + 1, minuend + 1, subtrahend + 1);
}

template <size_t L>
static inline std::array<uint32_t, L> & operator -= (std::array<uint32_t, L> &minuend, uint32_t subtrahend) noexcept {
	__sub<L>(minuend.data(), minuend.data(), subtrahend);
	return minuend;
}

template <size_t L>
static inline std::array<uint32_t, L> & operator -= (std::array<uint32_t, L> &minuend, const std::array<uint32_t, L> &subtrahend) noexcept {
	__sub<L>(minuend.data(), minuend.data(), subtrahend.data());
	return minuend;
}

template <size_t L>
static inline std::array<uint32_t, L> _pure operator - (const std::array<uint32_t, L> &minuend, uint32_t subtrahend) noexcept {
	std::array<uint32_t, L> difference;
	__sub<L>(difference.data(), minuend.data(), subtrahend);
	return difference;
}

template <size_t L>
static inline std::array<uint32_t, L> _pure operator - (const std::array<uint32_t, L> &minuend, const std::array<uint32_t, L> &subtrahend) noexcept {
	std::array<uint32_t, L> difference;
	__sub<L>(difference.data(), minuend.data(), subtrahend.data());
	return difference;
}

#else

#if SIZE_MAX >= UINT64_MAX
#	define LIMB_T uint32_t
#elif SIZE_MAX >= UINT32_MAX
#	define LIMB_T uint16_t
#else
#	define LIMB_T uint8_t
static inline uint8_t _const bswap(uint8_t v) noexcept { return v; }
#endif

#endif


template <size_t B>
class BigUInt {
	static_assert(B >= 128, "bit length must be >= 128");
	static_assert(B % 64 == 0, "bit length must be a multiple of 64");

public:
	typedef LIMB_T limb_t;
#undef LIMB_T

public:
	static constexpr size_t bit_length = B;
	static constexpr size_t limb_count = bit_length / 8 / sizeof(limb_t);
	static constexpr size_t word_count = bit_length / 8 / sizeof(uintptr_t);

public:
	union {
		std::array<uintptr_t, word_count> words;
		std::array<limb_t, limb_count> limbs;
	};

public:
	_pure explicit operator uintptr_t () const noexcept {
		return words[WORD(word_count, 0)];
	}

	BigUInt _pure operator + (limb_t addend) const noexcept {
		BigUInt sum;
		sum.limbs = limbs + addend;
		return sum;
	}

	BigUInt _pure operator + (const BigUInt &addend) const noexcept {
		BigUInt sum;
		sum.limbs = limbs + addend.limbs;
		return sum;
	}

	BigUInt _pure operator - (limb_t subtrahend) const noexcept {
		BigUInt difference;
		difference.limbs = limbs - subtrahend;
		return difference;
	}

	BigUInt _pure operator - (const BigUInt &subtrahend) const noexcept {
		BigUInt difference;
		difference.limbs = limbs - subtrahend.limbs;
		return difference;
	}

	BigUInt _pure operator << (unsigned shift) const noexcept {
		BigUInt result;
		result.words = words << shift;
		return result;
	}

	BigUInt _pure operator >> (unsigned shift) const noexcept {
		BigUInt result;
		result.words = words >> shift;
		return result;
	}

	BigUInt _pure operator ~ () const noexcept {
		BigUInt complement;
		complement.words = ~words;
		return complement;
	}

	BigUInt _pure operator & (const BigUInt &operand) const noexcept {
		BigUInt result;
		result.words = words & operand.words;
		return result;
	}

	BigUInt _pure operator | (const BigUInt &operand) const noexcept {
		BigUInt result;
		result.words = words | operand.words;
		return result;
	}

	BigUInt _pure operator ^ (const BigUInt &operand) const noexcept {
		BigUInt result;
		result.words = words ^ operand.words;
		return result;
	}

	BigUInt & operator = (uintptr_t value) noexcept {
		words.fill(0);
		words[WORD(word_count, 0)] = value;
		return *this;
	}

	BigUInt & operator += (limb_t addend) noexcept {
		limbs += addend;
		return *this;
	}

	BigUInt & operator += (const BigUInt &addend) noexcept {
		limbs += addend.limbs;
		return *this;
	}

	BigUInt & operator -= (limb_t subtrahend) noexcept {
		limbs -= subtrahend;
		return *this;
	}

	BigUInt & operator -= (const BigUInt &subtrahend) noexcept {
		limbs -= subtrahend.limbs;
		return *this;
	}

	BigUInt & operator <<= (unsigned shift) noexcept {
		words <<= shift;
		return *this;
	}

	BigUInt & operator >>= (unsigned shift) noexcept {
		words >>= shift;
		return *this;
	}

	BigUInt & operator &= (const BigUInt &operand) noexcept {
		words &= operand.words;
		return *this;
	}

	BigUInt & operator |= (const BigUInt &operand) noexcept {
		words |= operand.words;
		return *this;
	}

	BigUInt & operator ^= (const BigUInt &operand) noexcept {
		words ^= operand.words;
		return *this;
	}

	bool _pure operator == (const BigUInt &rhs) const noexcept { return __cmp(words, rhs.words) == 0; }
	bool _pure operator != (const BigUInt &rhs) const noexcept { return __cmp(words, rhs.words) != 0; }
	bool _pure operator < (const BigUInt &rhs) const noexcept { return __cmp(words, rhs.words) < 0; }
	bool _pure operator <= (const BigUInt &rhs) const noexcept { return __cmp(words, rhs.words) <= 0; }
	bool _pure operator > (const BigUInt &rhs) const noexcept { return __cmp(words, rhs.words) > 0; }
	bool _pure operator >= (const BigUInt &rhs) const noexcept { return __cmp(words, rhs.words) >= 0; }

	size_t _pure clz() const noexcept {
		return __clz(words);
	}

	size_t _pure ctz() const noexcept {
		return __ctz(words);
	}

	size_t _pure popcount() const noexcept {
		return __popcount(words);
	}

	size_t _pure parity() const noexcept {
		return __parity(words);
	}

	BigUInt & bswap() noexcept {
		__bswap(words);
		return *this;
	}

};

template <size_t B> static inline BigUInt<B> _pure bswap(BigUInt<B> v) noexcept { return v.bswap(); }

typedef BigUInt<128> uint128_t;
typedef BigUInt<256> uint256_t;
