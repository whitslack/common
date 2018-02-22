#include <cerrno>
#include <chrono>

#include <linux/futex.h>
#include <sys/syscall.h>

#include "compiler.h"


#if defined(__GNUC__) && defined(__OPTIMIZE__) && defined(__amd64__)

_nodiscard static inline _always_inline int futex(int *uaddr, int futex_op, int val) {
	register long rax __asm__ ("rax") = SYS_futex;
	__asm__ volatile ("syscall" : "+a" (rax) : "D" (uaddr), "S" (long(futex_op)), "d" (long(val)) : "rcx", "r11", "cc", "memory");
	return rax < 0 ? errno = static_cast<int>(-rax), -1 : static_cast<int>(rax);
}

_nodiscard static inline _always_inline int futex(int *uaddr, int futex_op, int val, const struct timespec *timeout) {
	register long rax __asm__ ("rax") = SYS_futex;
	register const struct timespec *r10 __asm__ ("r10") = timeout;
	__asm__ volatile ("syscall" : "+a" (rax) : "D" (uaddr), "S" (long(futex_op)), "d" (long(val)), "r" (r10) : "rcx", "r11", "cc", "memory");
	return rax < 0 ? errno = static_cast<int>(-rax), -1 : static_cast<int>(rax);
}

_nodiscard static inline _always_inline int futex(int *uaddr, int futex_op, int val, const struct timespec *timeout, int val3) {
	register long rax __asm__ ("rax") = SYS_futex;
	register const struct timespec *r10 __asm__ ("r10") = timeout;
	register long r9 __asm__ ("r9") = val3;
	__asm__ volatile ("syscall" : "+a" (rax) : "D" (uaddr), "S" (long(futex_op)), "d" (long(val)), "r" (r10), "r" (r9) : "rcx", "r11", "cc", "memory");
	return rax < 0 ? errno = static_cast<int>(-rax), -1 : static_cast<int>(rax);
}

_nodiscard static inline _always_inline int futex(int *uaddr, int futex_op, int val, uint32_t val2, int *uaddr2) {
	register long rax __asm__ ("rax") = SYS_futex;
	register unsigned long r10 __asm__ ("r10") = val2;
	register int *r8 __asm__ ("r8") = uaddr2;
	__asm__ volatile ("syscall" : "+a" (rax) : "D" (uaddr), "S" (long(futex_op)), "d" (long(val)), "r" (r10), "r" (r8) : "rcx", "r11", "cc", "memory");
	return rax < 0 ? errno = static_cast<int>(-rax), -1 : static_cast<int>(rax);
}

_nodiscard static inline _always_inline int futex(int *uaddr, int futex_op, int val, uint32_t val2, int *uaddr2, int val3) {
	register long rax __asm__ ("rax") = SYS_futex;
	register long r10 __asm__ ("r10") = val2;
	register int *r8 __asm__ ("r8") = uaddr2;
	register long r9 __asm__ ("r9") = val3;
	__asm__ volatile ("syscall" : "+a" (rax) : "D" (uaddr), "S" (long(futex_op)), "d" (long(val)), "r" (r10), "r" (r8), "r" (r9) : "rcx", "r11", "cc", "memory");
	return rax < 0 ? errno = static_cast<int>(-rax), -1 : static_cast<int>(rax);
}

_nodiscard static inline _always_inline int futex(int *uaddr, int futex_op, int val, int val3) {
	register long rax __asm__ ("rax") = SYS_futex;
	register long r9 __asm__ ("r9") = val3;
	__asm__ volatile ("syscall" : "+a" (rax) : "D" (uaddr), "S" (long(futex_op)), "d" (long(val)), "r" (r9) : "rcx", "r11", "cc", "memory");
	return rax < 0 ? errno = static_cast<int>(-rax), -1 : static_cast<int>(rax);
}

#else

#include <unistd.h>

// FUTEX_WAKE, FUTEX_FD
_nodiscard static inline int futex(int *uaddr, int futex_op, int val) {
	return static_cast<int>(::syscall(SYS_futex, uaddr, futex_op, val));
}

// FUTEX_WAIT
_nodiscard static inline int futex(int *uaddr, int futex_op, int val, const struct timespec *timeout) {
	return static_cast<int>(::syscall(SYS_futex, uaddr, futex_op, val, timeout));
}

// FUTEX_WAIT_BITSET
_nodiscard static inline int futex(int *uaddr, int futex_op, int val, const struct timespec *timeout, int val3) {
	return static_cast<int>(::syscall(SYS_futex, uaddr, futex_op, val, timeout, nullptr, val3));
}

// FUTEX_REQUEUE
_nodiscard static inline int futex(int *uaddr, int futex_op, int val, uint32_t val2, int *uaddr2) {
	return static_cast<int>(::syscall(SYS_futex, uaddr, futex_op, val, val2, uaddr2));
}

// FUTEX_CMP_REQUEUE, FUTEX_WAKE_OP
_nodiscard static inline int futex(int *uaddr, int futex_op, int val, uint32_t val2, int *uaddr2, int val3) {
	return static_cast<int>(::syscall(SYS_futex, uaddr, futex_op, val, val2, uaddr2, val3));
}

// FUTEX_WAKE_BITSET
_nodiscard static inline int futex(int *uaddr, int futex_op, int val, int val3) {
	return static_cast<int>(::syscall(SYS_futex, uaddr, futex_op, val, nullptr, nullptr, val3));
}

#endif


#undef linux
namespace linux {

unsigned futex_wake(int *uaddr, int count);

unsigned futex_wake_bitset(int *uaddr, int count, unsigned mask);

bool futex_wait(int *uaddr, int expect, const struct timespec *timeout = nullptr);

static inline bool futex_wait(int *uaddr, int expect, std::chrono::steady_clock::duration timeout) {
	struct timespec ts;
	ts.tv_sec = static_cast<std::time_t>(std::chrono::duration_cast<std::chrono::seconds>(timeout).count());
	ts.tv_nsec = static_cast<long>(std::chrono::duration_cast<std::chrono::nanoseconds>(timeout % std::chrono::seconds(1)).count());
	return futex_wait(uaddr, expect, &ts);
}

bool futex_wait_bitset(int *uaddr, int expect, unsigned mask, const struct timespec *deadline = nullptr);

bool futex_wait_bitset(int *uaddr, int expect, unsigned mask, std::chrono::system_clock::time_point deadline);

static inline bool futex_wait_bitset(int *uaddr, int expect, unsigned mask, std::chrono::steady_clock::time_point deadline) {
	struct timespec ts;
	ts.tv_sec = static_cast<std::time_t>(std::chrono::duration_cast<std::chrono::seconds>(deadline.time_since_epoch()).count());
	ts.tv_nsec = static_cast<long>(std::chrono::duration_cast<std::chrono::nanoseconds>(deadline.time_since_epoch() % std::chrono::seconds(1)).count());
	return futex_wait_bitset(uaddr, expect, mask, &ts);
}

} // namespace linux
