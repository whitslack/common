#include "timer.h"

#include <system_error>

#include "compiler.h"


namespace posix {


_Timer::_Timer(clockid_t clock_id, sigevent *sevp) {
	if (_unlikely(::timer_create(clock_id, sevp, &timer_id) < 0)) {
		throw std::system_error(errno, std::system_category(), "timer_create");
	}
}

_Timer::~_Timer() {
	if (_unlikely(::timer_delete(timer_id) < 0)) {
		throw std::system_error(errno, std::system_category(), "timer_delete");
	}
}

void _Timer::get(struct itimerspec &spec) const {
	if (_unlikely(::timer_gettime(timer_id, &spec) < 0)) {
		throw std::system_error(errno, std::system_category(), "timer_gettime");
	}
}

unsigned _Timer::overrun() const {
	int ret;
	if (_unlikely((ret = ::timer_getoverrun(timer_id)) < 0)) {
		throw std::system_error(errno, std::system_category(), "timer_getoverrun");
	}
	return static_cast<unsigned>(ret);
}

void _Timer::set(int flags, const struct itimerspec &new_spec, struct itimerspec *old_spec) {
	if (_unlikely(::timer_settime(timer_id, flags, &new_spec, old_spec) < 0)) {
		throw std::system_error(errno, std::system_category(), "timer_settime");
	}
}


} // namespace posix
