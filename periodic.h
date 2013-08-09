#include <atomic>
#include <functional>

#include "scheduler.h"


template <typename Clock = std::chrono::steady_clock>
class Periodic {

public:
	typedef Scheduler<Clock> scheduler_t;
	typedef typename scheduler_t::clock_t clock_t;
	typedef typename scheduler_t::time_point_t time_point_t;
	typedef typename scheduler_t::duration_t duration_t;

private:
	std::atomic_ulong sched_seq, exec_seq;
	std::atomic<scheduler_t *> scheduler;
	std::atomic<typename time_point_t::rep> deadline;
	std::atomic_flag running;

protected:
	Periodic() : sched_seq(0), exec_seq(0), scheduler(nullptr), deadline(clock_t::now().time_since_epoch().count()), running(ATOMIC_FLAG_INIT) { }
	explicit Periodic(scheduler_t *scheduler, time_point_t deadline = clock_t::now());

public:
	scheduler_t * get_scheduler() const { return scheduler; }
	bool set_scheduler(scheduler_t *scheduler);

	time_point_t get_deadline() const { return time_point_t(duration_t(deadline)); }
	bool set_deadline(time_point_t deadline);

	bool schedule(scheduler_t *scheduler, time_point_t deadline = clock_t::now());

protected:
	virtual bool work(time_point_t &deadline) = 0;

private:
	std::function<void (void) noexcept> make_task(unsigned long seq);

};
