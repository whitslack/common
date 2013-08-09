#include <cstdint>
#include <utility>

static inline std::pair<int32_t, int32_t> muldiv(int32_t multiplicand, int32_t multiplier, int32_t divisor) {
	std::pair<int32_t, int32_t> ret;
	__asm__ (
		"imull %3\n\tidivl %4"
		: "=a" (ret.first), "=&d" (ret.second)
		: "a" (multiplicand), "rm" (multiplier), "rm" (divisor)
	);
	return ret;
}

static inline std::pair<uint32_t, uint32_t> muldiv(uint32_t multiplicand, uint32_t multiplier, uint32_t divisor) {
	std::pair<uint32_t, uint32_t> ret;
	__asm__ (
		"mull %3\n\tdivl %4"
		: "=a" (ret.first), "=&d" (ret.second)
		: "a" (multiplicand), "rm" (multiplier), "rm" (divisor)
	);
	return ret;
}

static inline std::pair<int64_t, int64_t> muldiv(int64_t multiplicand, int64_t multiplier, int64_t divisor) {
	std::pair<int64_t, int64_t> ret;
	__asm__ (
		"imulq %3\n\tidivq %4"
		: "=a" (ret.first), "=&d" (ret.second)
		: "a" (multiplicand), "rm" (multiplier), "rm" (divisor)
	);
	return ret;
}

static inline std::pair<uint64_t, uint64_t> muldiv(uint64_t multiplicand, uint64_t multiplier, uint64_t divisor) {
	std::pair<uint64_t, uint64_t> ret;
	__asm__ (
		"mulq %3\n\tdivq %4"
		: "=a" (ret.first), "=&d" (ret.second)
		: "a" (multiplicand), "rm" (multiplier), "rm" (divisor)
	);
	return ret;
}
