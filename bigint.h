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
static inline typename std::enable_if<sizeof(LIMB_T) < sizeof(uintptr_t), void>::type __add(std::array<LIMB_T, L> &sum, const std::array<LIMB_T, L> &augend, LIMB_T addend) {
	intptr_t carry = addend;
	for (size_t i = 0; i < L; ++i) {
		sum[WORD(L, i)] = static_cast<LIMB_T>(carry += augend[WORD(L, i)]), carry >>= sizeof(LIMB_T) * 8;
	}
}

template <typename LIMB_T, size_t L>
static inline typename std::enable_if<sizeof(LIMB_T) < sizeof(uintptr_t), void>::type __add(std::array<LIMB_T, L> &sum, const std::array<LIMB_T, L> &augend, const std::array<LIMB_T, L> &addend) {
	intptr_t carry = 0;
	for (size_t i = 0; i < L; ++i) {
		sum[WORD(L, i)] = static_cast<LIMB_T>((carry += augend[WORD(L, i)]) += addend[WORD(L, i)]), carry >>= sizeof(LIMB_T) * 8;
	}
}

template <typename LIMB_T, size_t L>
static inline std::array<LIMB_T, L> & operator += (std::array<LIMB_T, L> &augend, LIMB_T addend) {
	if (addend != 0) {
		__add(augend, augend, addend);
	}
	return augend;
}

template <typename LIMB_T, size_t L>
static inline std::array<LIMB_T, L> & operator += (std::array<LIMB_T, L> &augend, const std::array<LIMB_T, L> &addend) {
	__add(augend, augend, addend);
	return augend;
}

template <typename LIMB_T, size_t L>
static inline std::array<LIMB_T, L> _pure operator + (const std::array<LIMB_T, L> &augend, LIMB_T addend) {
	std::array<LIMB_T, L> sum;
	__add(sum, augend, addend);
	return sum;
}

template <typename LIMB_T, size_t L>
static inline std::array<LIMB_T, L> _pure operator + (const std::array<LIMB_T, L> &augend, const std::array<LIMB_T, L> &addend) {
	std::array<LIMB_T, L> sum;
	__add(sum, augend, addend);
	return sum;
}

template <typename LIMB_T, size_t L>
static inline typename std::enable_if<sizeof(LIMB_T) < sizeof(uintptr_t), void>::type __sub(std::array<LIMB_T, L> &difference, const std::array<LIMB_T, L> &minuend, LIMB_T subtrahend) {
	intptr_t borrow = -subtrahend;
	for (size_t i = 0; i < L; ++i) {
		difference[WORD(L, i)] = static_cast<LIMB_T>(borrow += minuend[WORD(L, i)]), borrow >>= sizeof(LIMB_T) * 8;
	}
}

template <typename LIMB_T, size_t L>
static inline typename std::enable_if<sizeof(LIMB_T) < sizeof(uintptr_t), void>::type __sub(std::array<LIMB_T, L> &difference, const std::array<LIMB_T, L> &minuend, const std::array<LIMB_T, L> &subtrahend) {
	intptr_t borrow = 0;
	for (size_t i = 0; i < L; ++i) {
		difference[WORD(L, i)] = static_cast<LIMB_T>((borrow += minuend[WORD(L, i)]) -= subtrahend[WORD(L, i)]), borrow >>= sizeof(LIMB_T) * 8;
	}
}

template <typename LIMB_T, size_t L>
static inline std::array<LIMB_T, L> & operator -= (std::array<LIMB_T, L> &minuend, LIMB_T subtrahend) {
	if (subtrahend != 0) {
		__sub(minuend, minuend, subtrahend);
	}
	return minuend;
}

template <typename LIMB_T, size_t L>
static inline std::array<LIMB_T, L> & operator -= (std::array<LIMB_T, L> &minuend, const std::array<LIMB_T, L> &subtrahend) {
	__sub(minuend, minuend, subtrahend);
	return minuend;
}

template <typename LIMB_T, size_t L>
static inline std::array<LIMB_T, L> _pure operator - (const std::array<LIMB_T, L> &minuend, LIMB_T subtrahend) {
	std::array<LIMB_T, L> difference;
	__sub(difference, minuend, subtrahend);
	return difference;
}

template <typename LIMB_T, size_t L>
static inline std::array<LIMB_T, L> _pure operator - (const std::array<LIMB_T, L> &minuend, const std::array<LIMB_T, L> &subtrahend) {
	std::array<LIMB_T, L> difference;
	__sub(difference, minuend, subtrahend);
	return difference;
}

template <typename WORD_T, size_t W>
static inline typename std::enable_if<std::is_unsigned<WORD_T>::value, void>::type __shln(std::array<WORD_T, W> &result, const std::array<WORD_T, W> &operand, uint shift) {
	uint offset = static_cast<uint>(shift / (sizeof(WORD_T) * 8));
	if ((shift %= static_cast<uint>(sizeof(WORD_T) * 8)) == 0) {
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
static inline std::array<WORD_T, W> & operator <<= (std::array<WORD_T, W> &operand, uint shift) {
	if (shift != 0) {
		__shln(operand, operand, shift);
	}
	return operand;
}

template <typename WORD_T, size_t W>
static inline std::array<WORD_T, W> _pure operator << (const std::array<WORD_T, W> &operand, uint shift) {
	std::array<WORD_T, W> result;
	__shln(result, operand, shift);
	return result;
}

template <typename WORD_T, size_t W>
static inline typename std::enable_if<std::is_unsigned<WORD_T>::value, void>::type __shrn(std::array<WORD_T, W> &result, const std::array<WORD_T, W> &operand, uint shift) {
	uint offset = static_cast<uint>(shift / (sizeof(WORD_T) * 8));
	if ((shift %= static_cast<uint>(sizeof(WORD_T) * 8)) == 0) {
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
static inline std::array<WORD_T, W> & operator >>= (std::array<WORD_T, W> &operand, uint shift) {
	if (shift != 0) {
		__shrn(operand, operand, shift);
	}
	return operand;
}

template <typename WORD_T, size_t W>
static inline std::array<WORD_T, W> _pure operator >> (const std::array<WORD_T, W> &operand, uint shift) {
	std::array<WORD_T, W> result;
	__shrn(result, operand, shift);
	return result;
}

template <typename WORD_T, size_t W>
static inline void __com(std::array<WORD_T, W> &result, const std::array<WORD_T, W> &operand) {
	for (size_t i = 0; i < W; ++i) {
		result[WORD(W, i)] = ~operand[WORD(W, i)];
	}
}

template <typename WORD_T, size_t W>
static inline std::array<WORD_T, W> _pure operator ~ (const std::array<WORD_T, W> &operand) {
	std::array<WORD_T, W> result;
	__com(result, operand);
	return result;
}

template <typename WORD_T, size_t W>
static inline void __and(std::array<WORD_T, W> &result, const std::array<WORD_T, W> &operand1, const std::array<WORD_T, W> &operand2) {
	for (size_t i = 0; i < W; ++i) {
		result[WORD(W, i)] = operand1[WORD(W, i)] & operand2[WORD(W, i)];
	}
}

template <typename WORD_T, size_t W>
static inline std::array<WORD_T, W> & operator &= (std::array<WORD_T, W> &operand1, const std::array<WORD_T, W> &operand2) {
	__and(operand1, operand1, operand2);
	return operand1;
}

template <typename WORD_T, size_t W>
static inline std::array<WORD_T, W> _pure operator & (const std::array<WORD_T, W> &operand1, const std::array<WORD_T, W> &operand2) {
	std::array<WORD_T, W> result;
	__and(result, operand1, operand2);
	return result;
}

template <typename WORD_T, size_t W>
static inline void __or(std::array<WORD_T, W> &result, const std::array<WORD_T, W> &operand1, const std::array<WORD_T, W> &operand2) {
	for (size_t i = 0; i < W; ++i) {
		result[WORD(W, i)] = operand1[WORD(W, i)] | operand2[WORD(W, i)];
	}
}

template <typename WORD_T, size_t W>
static inline std::array<WORD_T, W> & operator |= (std::array<WORD_T, W> &operand1, const std::array<WORD_T, W> &operand2) {
	__or(operand1, operand1, operand2);
	return operand1;
}

template <typename WORD_T, size_t W>
static inline std::array<WORD_T, W> _pure operator | (const std::array<WORD_T, W> &operand1, const std::array<WORD_T, W> &operand2) {
	std::array<WORD_T, W> result;
	__or(result, operand1, operand2);
	return result;
}

template <typename WORD_T, size_t W>
static inline void __xor(std::array<WORD_T, W> &result, const std::array<WORD_T, W> &operand1, const std::array<WORD_T, W> &operand2) {
	for (size_t i = 0; i < W; ++i) {
		result[WORD(W, i)] = operand1[WORD(W, i)] ^ operand2[WORD(W, i)];
	}
}

template <typename WORD_T, size_t W>
static inline std::array<WORD_T, W> & operator ^= (std::array<WORD_T, W> &operand1, const std::array<WORD_T, W> &operand2) {
	__xor(operand1, operand1, operand2);
	return operand1;
}

template <typename WORD_T, size_t W>
static inline std::array<WORD_T, W> _pure operator ^ (const std::array<WORD_T, W> &operand1, const std::array<WORD_T, W> &operand2) {
	std::array<WORD_T, W> result;
	__xor(result, operand1, operand2);
	return result;
}

template <typename WORD_T, size_t W>
static inline typename std::make_signed<WORD_T>::type _pure __cmp(const std::array<WORD_T, W> &lhs, const std::array<WORD_T, W> &rhs) {
	for (size_t i = W; i-- > 0;) {
		auto c = lhs[WORD(W, i)] - rhs[WORD(W, i)];
		if (c != 0) {
			return c;
		}
	}
	return 0;
}

template <typename WORD_T, size_t W>
static inline size_t _pure __clz(const std::array<WORD_T, W> &operand) {
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
static inline size_t _pure __ctz(const std::array<WORD_T, W> &operand) {
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
static inline size_t _pure __popcount(const std::array<WORD_T, W> &operand) {
	size_t ret = 0;
	for (size_t i = 0; i < W; ++i) {
		ret += _popcount(operand[WORD(W, i)]);
	}
	return ret;
}

template <typename WORD_T, size_t W>
static inline size_t _pure __parity(const std::array<WORD_T, W> &operand) {
	size_t ret = 0;
	for (size_t i = 0; i < W; ++i) {
		ret ^= _parity(operand[WORD(W, i)]);
	}
	return ret;
}

template <typename WORD_T, size_t W>
static inline void __bswap(std::array<WORD_T, W> &operand) {
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
	static inline void __adc(WORD_T augend[]) { \
		__asm__ ("adc" SUFFIX " $0, %[augend]" : [augend] "+rm" (augend[0]) : "X" (augend[-1]) : "cc"); \
		__adc<L - 1>(augend + 1); \
	} \
	\
	template <> \
	inline void __adc<0>(WORD_T *) { \
	} \
	\
	template <size_t L> \
	static inline void __adc(WORD_T augend[], const WORD_T addend[]) { \
		__asm__ ("adc" SUFFIX " %[addend], %[augend]" : [augend] "+r,m" (augend[0]) : [addend] "irm,ir" (addend[0]), "X,X" (augend[-1]) : "cc"); \
		__adc<L - 1>(augend + 1, addend + 1); \
	} \
	\
	template <> \
	inline void __adc<0>(WORD_T *, const WORD_T *) { \
	} \
	\
	template <size_t L> \
	static inline void __add(WORD_T augend[], WORD_T addend) { \
		__asm__ ("add" SUFFIX " %[addend], %[augend]" : [augend] "+r,m" (augend[0]) : [addend] "irm,ir" (addend) : "cc"); \
		__adc<L - 1>(augend + 1); \
	} \
	\
	template <size_t L> \
	static inline void __add(WORD_T augend[], const WORD_T addend[]) { \
		__asm__ ("add" SUFFIX " %[addend], %[augend]" : [augend] "+r,m" (augend[0]) : [addend] "irm,ir" (addend[0]) : "cc"); \
		__adc<L - 1>(augend + 1, addend + 1); \
	} \
	\
	template <size_t L> \
	static inline std::array<WORD_T, L> & operator += (std::array<WORD_T, L> &augend, WORD_T addend) { \
		__add<L>(augend.data(), addend); \
		return augend; \
	} \
	\
	template <size_t L> \
	static inline std::array<WORD_T, L> & operator += (std::array<WORD_T, L> &augend, const std::array<WORD_T, L> &addend) { \
		__add<L>(augend.data(), addend.data()); \
		return augend; \
	} \
	\
	template <size_t L> \
	static inline std::array<WORD_T, L> _pure operator + (std::array<WORD_T, L> augend, WORD_T addend) { \
		return augend += addend; \
	} \
	\
	template <size_t L> \
	static inline std::array<WORD_T, L> _pure operator + (std::array<WORD_T, L> augend, const std::array<WORD_T, L> &addend) { \
		return augend += addend; \
	} \
	\
	template <size_t L> \
	static inline void __sbb(WORD_T minuend[]) { \
		__asm__ ("sbb" SUFFIX " $0, %[minuend]" : [minuend] "+rm" (minuend[0]) : "X" (minuend[-1]) : "cc"); \
		__sbb<L - 1>(minuend + 1); \
	} \
	\
	template <> \
	inline void __sbb<0>(WORD_T *) { \
	} \
	\
	template <size_t L> \
	static inline void __sbb(WORD_T minuend[], const WORD_T subtrahend[]) { \
		__asm__ ("sbb" SUFFIX " %[subtrahend], %[minuend]" : [minuend] "+r,m" (minuend[0]) : [subtrahend] "irm,ir" (subtrahend[0]), "X,X" (minuend[-1]) : "cc"); \
		__sbb<L - 1>(minuend + 1, subtrahend + 1); \
	} \
	\
	template <> \
	inline void __sbb<0>(WORD_T *, const WORD_T *) { \
	} \
	\
	template <size_t L> \
	static inline void __sub(WORD_T minuend[], WORD_T subtrahend) { \
		__asm__ ("sub" SUFFIX " %[subtrahend], %[minuend]" : [minuend] "+r,m" (minuend[0]) : [subtrahend] "irm,ir" (subtrahend) : "cc"); \
		__sbb<L - 1>(minuend + 1); \
	} \
	\
	template <size_t L> \
	static inline void __sub(WORD_T minuend[], const WORD_T subtrahend[]) { \
		__asm__ ("sub" SUFFIX " %[subtrahend], %[minuend]" : [minuend] "+r,m" (minuend[0]) : [subtrahend] "irm,ir" (subtrahend[0]) : "cc"); \
		__sbb<L - 1>(minuend + 1, subtrahend + 1); \
	} \
	\
	template <size_t L> \
	static inline std::array<WORD_T, L> & operator -= (std::array<WORD_T, L> &minuend, WORD_T subtrahend) { \
		__sub<L>(minuend.data(), subtrahend); \
		return minuend; \
	} \
	\
	template <size_t L> \
	static inline std::array<WORD_T, L> & operator -= (std::array<WORD_T, L> &minuend, const std::array<WORD_T, L> &subtrahend) { \
		__sub<L>(minuend.data(), subtrahend.data()); \
		return minuend; \
	} \
	\
	template <size_t L> \
	static inline std::array<WORD_T, L> _pure operator - (std::array<WORD_T, L> minuend, WORD_T subtrahend) { \
		return minuend -= subtrahend; \
	} \
	\
	template <size_t L> \
	static inline std::array<WORD_T, L> _pure operator - (std::array<WORD_T, L> minuend, const std::array<WORD_T, L> &subtrahend) { \
		return minuend -= subtrahend; \
	} \
	\
	template <size_t L> \
	static inline void __rcl(WORD_T operand[]) { \
		__asm__ ("rcl" SUFFIX " %[operand]" : [operand] "+rm" (operand[0]) : "X" (operand[-1]) : "cc"); \
		__rcl<L - 1>(operand + 1); \
	} \
	\
	template <> \
	inline void __rcl<0>(WORD_T *) { \
	} \
	\
	template <size_t L> \
	static inline void __shl(WORD_T operand[]) { \
		__asm__ ("shl" SUFFIX " %[operand]" : [operand] "+rm" (operand[0]) : : "cc"); \
		__rcl<L - 1>(operand + 1); \
	} \
	\
	template <size_t L> \
	static inline std::array<WORD_T, L> & operator <<= (std::array<WORD_T, L> &operand, uint shift) { \
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
	static inline std::array<WORD_T, L> _pure operator << (const std::array<WORD_T, L> &operand, uint shift) { \
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
	static inline void __rcr(WORD_T operand[]) { \
		__asm__ ("rcr" SUFFIX " %[operand]" : [operand] "+rm" (operand[L - 1]) : "X" (operand[L]) : "cc"); \
		__rcr<L - 1>(operand); \
	} \
	\
	template <> \
	inline void __rcr<0>(WORD_T *) { \
	} \
	\
	template <size_t L> \
	static inline void __shr(WORD_T operand[]) { \
		__asm__ ("shr" SUFFIX " %[operand]" : [operand] "+rm" (operand[L - 1]) : : "cc"); \
		__rcr<L - 1>(operand); \
	} \
	\
	template <size_t L> \
	static inline std::array<WORD_T, L> & operator >>= (std::array<WORD_T, L> &operand, uint shift) { \
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
	static inline std::array<WORD_T, L> _pure operator >> (const std::array<WORD_T, L> &operand, uint shift) { \
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

#else

#if SIZE_MAX >= UINT64_MAX
#	define LIMB_T uint32_t
#elif SIZE_MAX >= UINT32_MAX
#	define LIMB_T uint16_t
#else
#	define LIMB_T uint8_t
static inline uint8_t bswap(uint8_t v) { return v; }
static inline uint8_t htobe(uint8_t v) { return v; }
static inline uint8_t betoh(uint8_t v) { return v; }
static inline uint8_t htole(uint8_t v) { return v; }
static inline uint8_t letoh(uint8_t v) { return v; }
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
	explicit operator uintptr_t () const _pure {
		return words[WORD(word_count, 0)];
	}

	BigUInt operator + (limb_t addend) const _pure {
		BigUInt sum;
		sum.limbs = limbs + addend;
		return sum;
	}

	BigUInt operator + (const BigUInt &addend) const _pure {
		BigUInt sum;
		sum.limbs = limbs + addend.limbs;
		return sum;
	}

	BigUInt operator - (limb_t subtrahend) const _pure {
		BigUInt difference;
		difference.limbs = limbs - subtrahend;
		return difference;
	}

	BigUInt operator - (const BigUInt &subtrahend) const _pure {
		BigUInt difference;
		difference.limbs = limbs - subtrahend.limbs;
		return difference;
	}

	BigUInt operator << (uint shift) const _pure {
		BigUInt result;
		result.words = words << shift;
		return result;
	}

	BigUInt operator >> (uint shift) const _pure {
		BigUInt result;
		result.words = words >> shift;
		return result;
	}

	BigUInt operator ~ () const _pure {
		BigUInt complement;
		complement.words = ~words;
		return complement;
	}

	BigUInt operator & (const BigUInt &operand) const _pure {
		BigUInt result;
		result.words = words & operand.words;
		return result;
	}

	BigUInt operator | (const BigUInt &operand) const _pure {
		BigUInt result;
		result.words = words | operand.words;
		return result;
	}

	BigUInt operator ^ (const BigUInt &operand) const _pure {
		BigUInt result;
		result.words = words ^ operand.words;
		return result;
	}

	BigUInt & operator = (uintptr_t value) {
		words.fill(0);
		words[WORD(word_count, 0)] = value;
		return *this;
	}

	BigUInt & operator += (limb_t addend) {
		limbs += addend;
		return *this;
	}

	BigUInt & operator += (const BigUInt &addend) {
		limbs += addend.limbs;
		return *this;
	}

	BigUInt & operator -= (limb_t subtrahend) {
		limbs -= subtrahend;
		return *this;
	}

	BigUInt & operator -= (const BigUInt &subtrahend) {
		limbs -= subtrahend.limbs;
		return *this;
	}

	BigUInt & operator <<= (uint shift) {
		words <<= shift;
		return *this;
	}

	BigUInt & operator >>= (uint shift) {
		words >>= shift;
		return *this;
	}

	BigUInt & operator &= (const BigUInt &operand) {
		words &= operand.words;
		return *this;
	}

	BigUInt & operator |= (const BigUInt &operand) {
		words |= operand.words;
		return *this;
	}

	BigUInt & operator ^= (const BigUInt &operand) {
		words ^= operand.words;
		return *this;
	}

	bool operator == (const BigUInt &rhs) const _pure { return __cmp(words, rhs.words) == 0; }
	bool operator != (const BigUInt &rhs) const _pure { return __cmp(words, rhs.words) != 0; }
	bool operator < (const BigUInt &rhs) const _pure { return __cmp(words, rhs.words) < 0; }
	bool operator <= (const BigUInt &rhs) const _pure { return __cmp(words, rhs.words) <= 0; }
	bool operator > (const BigUInt &rhs) const _pure { return __cmp(words, rhs.words) > 0; }
	bool operator >= (const BigUInt &rhs) const _pure { return __cmp(words, rhs.words) >= 0; }

	size_t clz() const _pure {
		return __clz(words);
	}

	size_t ctz() const _pure {
		return __ctz(words);
	}

	size_t popcount() const _pure {
		return __popcount(words);
	}

	size_t parity() const _pure {
		return __parity(words);
	}

	BigUInt & bswap() {
		__bswap(words);
		return *this;
	}

#if BYTE_ORDER == BIG_ENDIAN
	BigUInt & htobe() { return *this; }
	BigUInt & betoh() { return *this; }
	BigUInt & htole() { __bswap(words); return *this; }
	BigUInt & letoh() { __bswap(words); return *this; }
#elif BYTE_ORDER == LITTLE_ENDIAN
	BigUInt & htobe() { __bswap(words); return *this; }
	BigUInt & betoh() { __bswap(words); return *this; }
	BigUInt & htole() { return *this; }
	BigUInt & letoh() { return *this; }
#endif

};

template <size_t B> static inline BigUInt<B> _pure bswap(BigUInt<B> v) { return v.bswap(); }
template <size_t B> static inline BigUInt<B> _pure htobe(BigUInt<B> v) { return v.htobe(); }
template <size_t B> static inline BigUInt<B> _pure betoh(BigUInt<B> v) { return v.betoh(); }
template <size_t B> static inline BigUInt<B> _pure htole(BigUInt<B> v) { return v.htole(); }
template <size_t B> static inline BigUInt<B> _pure letoh(BigUInt<B> v) { return v.letoh(); }

typedef BigUInt<128> uint128_t;
typedef BigUInt<256> uint256_t;
