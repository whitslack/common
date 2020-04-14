#pragma once

#include <chrono>

#include <time.h>

#include "auto_cast.h"
#include "compiler.h"


namespace posix {


static constexpr struct timeval _const duration_to_timeval(std::chrono::microseconds micros) noexcept {
	struct timeval ret {
		.tv_sec = auto_cast(std::chrono::duration_cast<std::chrono::seconds>(micros).count()),
		.tv_usec = auto_cast((micros % std::chrono::seconds(1)).count())
	};
	if (ret.tv_usec < 0) {
		ret.tv_usec += 1000000;
		--ret.tv_sec;
	}
	return ret;
}

static constexpr std::chrono::microseconds _const timeval_to_duration(const struct timeval &tv) noexcept {
	return std::chrono::seconds(tv.tv_sec) + std::chrono::microseconds(tv.tv_usec);
}

static constexpr struct timespec _const duration_to_timespec(std::chrono::nanoseconds nanos) noexcept {
	struct timespec ret {
		.tv_sec = auto_cast(std::chrono::duration_cast<std::chrono::seconds>(nanos).count()),
		.tv_nsec = auto_cast((nanos % std::chrono::seconds(1)).count())
	};
	if (ret.tv_nsec < 0) {
		ret.tv_nsec += 1000000000;
		--ret.tv_sec;
	}
	return ret;
}

static constexpr std::chrono::nanoseconds _const timespec_to_duration(const struct timespec &ts) noexcept {
	return std::chrono::seconds(ts.tv_sec) + std::chrono::nanoseconds(ts.tv_nsec);
}


} // namespace posix
