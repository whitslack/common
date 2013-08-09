#include <chrono>
#include <stdexcept>

#include <pthread.h>

namespace compat {


class ReadLock {
	friend class RWLock;

private:
	pthread_rwlock_t * const rwlock;

private:
	explicit ReadLock(pthread_rwlock_t *rwlock) : rwlock(rwlock) { }

public:
	void lock();
	bool try_lock();
	bool try_lock_for(const timespec &timeout_duration);
	template <typename Rep, typename Period>
	bool try_lock_for(const std::chrono::duration<Rep, Period> &timeout_duration) {
		std::chrono::nanoseconds nanos = timeout_duration;
		timespec ts;
		ts.tv_sec = nanos.count() / 1000000000;
		ts.tv_nsec = nanos.count() % 1000000000;
		return this->try_lock_for(ts);
	}
	template <typename Clock, typename Duration>
	bool try_lock_until(const std::chrono::time_point<Clock, Duration> &timeout_time) {
		return this->try_lock_for(timeout_time - Clock::now());
	}
	void unlock();

};


class WriteLock {
	friend class RWLock;

private:
	pthread_rwlock_t * const rwlock;

private:
	explicit WriteLock(pthread_rwlock_t *rwlock) : rwlock(rwlock) { }

public:
	void lock();
	bool try_lock();
	bool try_lock_for(const timespec &timeout_duration);
	template <typename Rep, typename Period>
	bool try_lock_for(const std::chrono::duration<Rep, Period> &timeout_duration) {
		std::chrono::nanoseconds nanos = timeout_duration;
		timespec ts;
		ts.tv_sec = nanos.count() / 1000000000;
		ts.tv_nsec = nanos.count() % 1000000000;
		return this->try_lock_for(ts);
	}
	template <typename Clock, typename Duration>
	bool try_lock_until(const std::chrono::time_point<Clock, Duration> &timeout_time) {
		return this->try_lock_for(timeout_time - Clock::now());
	}
	void unlock();

};


class RWLock {

private:
	pthread_rwlock_t rwlock;

public:
	RWLock();
	~RWLock();
	ReadLock read_lock() { return ReadLock(&rwlock); }
	WriteLock write_lock() { return WriteLock(&rwlock); }

private:
	RWLock(const RWLock &) = delete;
	RWLock & operator = (const RWLock &) = delete;

};


} // namespace compat
