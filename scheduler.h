#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>

#include "compiler.h"


template <typename Clock>
class Scheduler {

public:
	typedef Clock clock_t;
	typedef typename clock_t::time_point time_point_t;
	typedef typename clock_t::duration duration_t;

private:
	struct Work {
		time_point_t deadline;
		std::function<void (void) /* noexcept */> task;
		bool operator > (const Work &other) const { return deadline > other.deadline; }
	};

private:
	std::mutex mutex;
	std::condition_variable condition;
	std::priority_queue<Work, std::vector<Work>, std::greater<Work>> queue;

public:
	void run() _noreturn;

	template <typename T>
	void call_at(time_point_t deadline, T &&task) {
		std::lock_guard<std::mutex> lock(mutex);
		if (queue.empty() || deadline < queue.top().deadline) {
			condition.notify_one();
		}
		queue.push({ deadline, std::forward<T>(task) });
	}

	template <typename T>
	void call_after(duration_t delay, T &&task) {
		this->call_at(Clock::now() + delay, std::forward<T>(task));
	}

};
