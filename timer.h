#include <chrono>
#include <utility>

#include <time.h>

namespace posix {


class _Timer {

protected:
	template <typename Clock>
	struct clock_id { };

private:
	timer_t timer_id;

public:
	_Timer(clockid_t clock_id, struct sigevent *sevp);
	~_Timer();

private:
	_Timer(const _Timer &) = delete;

public:
	void get(struct itimerspec &spec) const;

	unsigned overrun() const;

	void set(int flags, const struct itimerspec &new_spec, struct itimerspec *old_spec = nullptr);

protected:
	void set(int flags, std::chrono::nanoseconds interval, std::chrono::nanoseconds value, struct itimerspec *old_spec) {
		struct itimerspec new_spec;
		new_spec.it_interval.tv_sec = static_cast<std::time_t>(std::chrono::duration_cast<std::chrono::seconds>(interval).count());
		new_spec.it_interval.tv_nsec = static_cast<long>((interval % std::chrono::seconds(1)).count());
		new_spec.it_value.tv_sec = static_cast<std::time_t>(std::chrono::duration_cast<std::chrono::seconds>(value).count());
		new_spec.it_value.tv_nsec = static_cast<long>((value % std::chrono::seconds(1)).count());
		this->set(flags, new_spec, old_spec);
	}

};


template <>
struct _Timer::clock_id<std::chrono::system_clock> : std::integral_constant<clockid_t, CLOCK_REALTIME> {
};

template <>
struct _Timer::clock_id<std::chrono::steady_clock> : std::integral_constant<clockid_t, CLOCK_MONOTONIC> {
};


template <typename Clock = std::chrono::steady_clock>
class Timer : public _Timer {

public:
	explicit Timer(struct sigevent *sevp = nullptr) : _Timer(clock_id<Clock>(), sevp) { }

public:
	void set(typename Clock::time_point abs_value, typename Clock::duration interval = Clock::duration::zero(), struct itimerspec *old_spec = nullptr) {
		this->_Timer::set(TIMER_ABSTIME, interval, abs_value.time_since_epoch(), old_spec);
	}

	void set(typename Clock::duration rel_value, typename Clock::duration interval = Clock::duration::zero(), struct itimerspec *old_spec = nullptr) {
		this->_Timer::set(0, interval, rel_value, old_spec);
	}

	void clear(struct itimerspec *old_spec = nullptr) {
		this->set(Clock::duration::zero(), Clock::duration::zero(), old_spec);
	}

};


} // namespace posix
