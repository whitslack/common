#include "periodic.h"

#include <thread>


template <typename Clock>
Periodic<Clock>::Periodic(scheduler_t *scheduler, time_point_t deadline) : sched_seq(1), exec_seq(0), scheduler(scheduler), deadline(deadline.time_since_epoch().count()), running(ATOMIC_FLAG_INIT) {
	if (scheduler) {
		scheduler->call_at(deadline, this->make_task(1));
	}
}

template <typename Clock>
bool Periodic<Clock>::set_scheduler(scheduler_t *scheduler) {
	scheduler_t *old_scheduler = this->scheduler;
	while (scheduler != old_scheduler) {
		unsigned long seq = ++sched_seq;
		if (this->scheduler.compare_exchange_weak(old_scheduler, scheduler)) {
			if (scheduler) {
				scheduler->call_at(this->get_deadline(), this->make_task(seq));
			}
			return true;
		}
	}
	return false;
}

template <typename Clock>
bool Periodic<Clock>::set_deadline(time_point_t deadline) {
	typename time_point_t::rep deadline_rep = deadline.time_since_epoch().count();
	typename time_point_t::rep old_deadline_rep = this->deadline;
	for (;;) {
		if (deadline_rep < old_deadline_rep || exec_seq == sched_seq) {
			unsigned long seq = ++sched_seq;
			if (this->deadline.compare_exchange_weak(old_deadline_rep, deadline_rep)) {
				scheduler_t *scheduler = this->scheduler;
				if (scheduler) {
					scheduler->call_at(deadline, this->make_task(seq));
				}
				return true;
			}
		}
		else if (deadline_rep > old_deadline_rep) {
			if (this->deadline.compare_exchange_weak(old_deadline_rep, deadline_rep)) {
				return true;
			}
		}
		else {
			return false;
		}
	}
}

template <typename Clock>
bool Periodic<Clock>::schedule(scheduler_t *scheduler, time_point_t deadline) {
	scheduler_t *old_scheduler;
	typename time_point_t::rep deadline_rep = deadline.time_since_epoch().count();
	typename time_point_t::rep old_deadline_rep;
	for (unsigned long old_seq = sched_seq, new_seq;; old_seq = new_seq) {
		old_scheduler = this->scheduler;
		old_deadline_rep = this->deadline;
		if ((new_seq = sched_seq) == old_seq) {
			break;
		}
	}
	for (;;) {
		if (scheduler != old_scheduler || deadline_rep < old_deadline_rep || exec_seq == sched_seq) {
			unsigned long seq = ++sched_seq;
			if (this->scheduler.compare_exchange_weak(old_scheduler, scheduler) && this->deadline.compare_exchange_weak(old_deadline_rep, deadline_rep)) {
				if (scheduler) {
					scheduler->call_at(deadline, this->make_task(seq));
				}
				return true;
			}
		}
		else if (deadline_rep > old_deadline_rep) {
			if (this->deadline.compare_exchange_weak(old_deadline_rep, deadline_rep)) {
				return true;
			}
		}
		else {
			return false;
		}
	}
}

template <typename Clock>
std::function<void (void) noexcept> Periodic<Clock>::make_task(unsigned long seq) {
	return [this, seq]() noexcept {
		if (sched_seq == seq) {
			scheduler_t *scheduler = this->scheduler;
			time_point_t deadline = this->get_deadline();
			if (sched_seq == seq) {
				if (deadline > clock_t::now()) {
					scheduler->call_at(deadline, this->make_task(seq));
				}
				else if (!running.test_and_set()) {
					exec_seq = seq;
					std::thread([this, seq, scheduler, deadline]() {
						time_point_t new_deadline = deadline;
						bool reschedule;
						try {
							reschedule = this->work(new_deadline);
						}
						catch (...) {
							reschedule = false;
						}
						if (reschedule && sched_seq == seq) {
							this->set_deadline(new_deadline);
						}
						running.clear();
					}).detach();
				}
			}
		}
	};
}
