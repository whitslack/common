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
			if (condition.wait_until(lock, work.deadline) == std::cv_status::timeout) {
				lock.unlock();
				(*work.task)();
				lock.lock();
				queue.pop();
			}
		}
	}
}
