#include <cstdint>
#include <utility>

#include "compiler.h"


static inline std::pair<int32_t, int32_t> _const muldiv(int32_t multiplicand, int32_t multiplier, int32_t divisor) noexcept {
	int32_t eax, edx;
	__asm__ (".ifnc %2,%%eax\n\timull %2\n\t.else\n\timull %3\n\t.endif" : "=a,a" (eax), "=d,d" (edx) : "?0,?rm" (multiplicand), "?rm,?0" (multiplier) : "cc");
	__asm__ ("idivl %2" : "+a" (eax), "+d" (edx) : "rm" (divisor) : "cc");
	return { eax, edx };
}

static inline std::pair<uint32_t, uint32_t> _const muldiv(uint32_t multiplicand, uint32_t multiplier, uint32_t divisor) noexcept {
	uint32_t eax, edx;
	__asm__ (".ifnc %2,%%eax\n\tmull %2\n\t.else\n\tmull %3\n\t.endif" : "=a,a" (eax), "=d,d" (edx) : "?0,?rm" (multiplicand), "?rm,?0" (multiplier) : "cc");
	__asm__ ("divl %2" : "+a" (eax), "+d" (edx) : "rm" (divisor) : "cc");
	return { eax, edx };
}

static inline std::pair<int64_t, int64_t> _const muldiv(int64_t multiplicand, int64_t multiplier, int64_t divisor) noexcept {
	int64_t rax, rdx;
	__asm__ (".ifnc %2,%%rax\n\timulq %2\n\t.else\n\timulq %3\n\t.endif" : "=a,a" (rax), "=d,d" (rdx) : "?0,?rm" (multiplicand), "?rm,?0" (multiplier) : "cc");
	__asm__ ("idivq %2" : "+a" (rax), "+d" (rdx) : "rm" (divisor) : "cc");
	return { rax, rdx };
}

static inline std::pair<uint64_t, uint64_t> _const muldiv(uint64_t multiplicand, uint64_t multiplier, uint64_t divisor) noexcept {
	uint64_t rax, rdx;
	__asm__ (".ifnc %2,%%rax\n\tmulq %2\n\t.else\n\tmulq %3\n\t.endif" : "=a,a" (rax), "=d,d" (rdx) : "?0,?rm" (multiplicand), "?rm,?0" (multiplier) : "cc");
	__asm__ ("divq %2" : "+a" (rax), "+d" (rdx) : "rm" (divisor) : "cc");
	return { rax, rdx };
}

static inline std::pair<int32_t, int32_t> _const muladddiv(int32_t multiplicand, int32_t multiplier, int32_t addend, int32_t divisor) noexcept {
	int32_t eax, edx;
	__asm__ (".ifnc %2,%%eax\n\timull %2\n\t.else\n\timull %3\n\t.endif" : "=a,a" (eax), "=d,d" (edx) : "?0,?rm" (multiplicand), "?rm,?0" (multiplier) : "cc");
	__asm__ ("addl %1, %0" : "+a" (eax) : "g" (addend) : "cc");
	__asm__ ("adcl %1, %0" : "+d" (edx) : "g" (addend >> 31), "X" (eax) : "cc");
	__asm__ ("idivl %2" : "+a" (eax), "+d" (edx) : "rm" (divisor) : "cc");
	return { eax, edx };
}

static inline std::pair<uint32_t, uint32_t> _const muladddiv(uint32_t multiplicand, uint32_t multiplier, uint32_t addend, uint32_t divisor) noexcept {
	uint32_t eax, edx;
	__asm__ (".ifnc %2,%%eax\n\tmull %2\n\t.else\n\tmull %3\n\t.endif" : "=a,a" (eax), "=d,d" (edx) : "?0,?rm" (multiplicand), "?rm,?0" (multiplier) : "cc");
	__asm__ ("addl %2, %0\n\tadcl $0, %1" : "+a" (eax), "+d" (edx) : "g" (addend) : "cc");
	__asm__ ("divl %2" : "+a" (eax), "+d" (edx) : "rm" (divisor) : "cc");
	return { eax, edx };
}

static inline std::pair<int64_t, int64_t> _const muladddiv(int64_t multiplicand, int64_t multiplier, int64_t addend, int64_t divisor) noexcept {
	int64_t rax, rdx;
	__asm__ (".ifnc %2,%%rax\n\timulq %2\n\t.else\n\timulq %3\n\t.endif" : "=a,a" (rax), "=d,d" (rdx) : "?0,?rm" (multiplicand), "?rm,?0" (multiplier) : "cc");
	__asm__ ("addq %1, %0" : "+a" (rax) : "g" (addend) : "cc");
	__asm__ ("adcq %1, %0" : "+d" (rdx) : "g" (addend >> 63), "X" (rax) : "cc");
	__asm__ ("idivq %2" : "+a" (rax), "+d" (rdx) : "rm" (divisor) : "cc");
	return { rax, rdx };
}

static inline std::pair<uint64_t, uint64_t> _const muladddiv(uint64_t multiplicand, uint64_t multiplier, uint64_t addend, uint64_t divisor) noexcept {
	uint64_t rax, rdx;
	__asm__ (".ifnc %2,%%rax\n\tmulq %2\n\t.else\n\tmulq %3\n\t.endif" : "=a,a" (rax), "=d,d" (rdx) : "?0,?rm" (multiplicand), "?rm,?0" (multiplier) : "cc");
	__asm__ ("addq %2, %0\n\tadcq $0, %1" : "+a" (rax), "+d" (rdx) : "g" (addend) : "cc");
	__asm__ ("divq %2" : "+a" (rax), "+d" (rdx) : "rm" (divisor) : "cc");
	return { rax, rdx };
}
