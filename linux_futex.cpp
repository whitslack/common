#include "linux_futex.h"

#include <system_error>


namespace linux {

unsigned futex_wake(int *uaddr, int count) {
	int ret;
	if ((ret = ::futex(uaddr, FUTEX_WAKE_PRIVATE, count)) < 0) {
		throw std::system_error(errno, std::system_category(), "futex");
	}
	return static_cast<unsigned>(ret);
}

unsigned futex_wake_bitset(int *uaddr, int count, unsigned mask) {
	int ret;
	if ((ret = ::futex(uaddr, FUTEX_WAKE_BITSET_PRIVATE, count, mask)) < 0) {
		throw std::system_error(errno, std::system_category(), "futex");
	}
	return static_cast<unsigned>(ret);
}

bool futex_wait(int *uaddr, int expect, const struct timespec *timeout) {
	if (::futex(uaddr, FUTEX_WAIT_PRIVATE, expect, timeout) < 0) {
		if (errno == EAGAIN || errno == EINTR || errno == ETIMEDOUT) {
			return false;
		}
		throw std::system_error(errno, std::system_category(), "futex");
	}
	return true;
}

bool futex_wait_bitset(int *uaddr, int expect, unsigned mask, const struct timespec *deadline) {
	if (::futex(uaddr, FUTEX_WAIT_BITSET_PRIVATE, expect, deadline, mask) < 0) {
		if (errno == EAGAIN || errno == EINTR || errno == ETIMEDOUT) {
			return false;
		}
		throw std::system_error(errno, std::system_category(), "futex");
	}
	return true;
}

bool futex_wait_bitset(int *uaddr, int expect, unsigned mask, std::chrono::system_clock::time_point deadline) {
	struct timespec ts;
	ts.tv_sec = static_cast<std::time_t>(std::chrono::duration_cast<std::chrono::seconds>(deadline.time_since_epoch()).count());
	ts.tv_nsec = static_cast<long>(std::chrono::duration_cast<std::chrono::nanoseconds>(deadline.time_since_epoch() % std::chrono::seconds(1)).count());
	if (::futex(uaddr, FUTEX_WAIT_BITSET_PRIVATE | FUTEX_CLOCK_REALTIME, expect, &ts, mask) < 0) {
		if (errno == EAGAIN || errno == EINTR || errno == ETIMEDOUT) {
			return false;
		}
		throw std::system_error(errno, std::system_category(), "futex");
	}
	return true;
}

} // namespace linux
