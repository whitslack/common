#include "scheduler.h"


template <typename Clock>
void Scheduler<Clock>::run() {
	std::unique_lock<std::mutex> lock(mutex);
	for (;;) {
		if (queue.empty()) {
			condition.wait(lock);
		}
		else {
			auto &work = queue.top();
			if (clock_t::now() < work.deadline) {
				condition.wait_until(lock, work.deadline);
			}
			else {
				auto task = std::move(work.task);
				queue.pop();
				lock.unlock();
				task();
				lock.lock();
			}
		}
	}
}
