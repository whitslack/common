#include <condition_variable>
#include <mutex>
#include <thread>

class ProgressBar {

private:
	std::mutex mutex;
	std::condition_variable condition;
	uint_least64_t pos, size, delta;
	volatile bool finished;
	std::thread display_thread;

public:
	ProgressBar(uint_least64_t size);
	~ProgressBar();

	void update(uint_least64_t delta);

private:
	void display();

};
