#include "rwlock.h"

#include <system_error>

namespace posix {


void ReadLock::lock() {
	int error;
	if ((error = ::pthread_rwlock_rdlock(rwlock)) != 0) {
		throw std::system_error(error, std::system_category(), "pthread_rwlock_rdlock");
	}
}

bool ReadLock::try_lock() {
	int error;
	if ((error = ::pthread_rwlock_tryrdlock(rwlock)) != 0) {
		if (error == EBUSY) {
			return false;
		}
		throw std::system_error(error, std::system_category(), "pthread_rwlock_tryrdlock");
	}
	return true;
}

bool ReadLock::try_lock_for(const timespec &timeout_duration) {
	int error;
	if ((error = ::pthread_rwlock_timedrdlock(rwlock, &timeout_duration)) != 0) {
		if (error == ETIMEDOUT) {
			return false;
		}
		throw std::system_error(error, std::system_category(), "pthread_rwlock_timedrdlock");
	}
	return true;
}

void ReadLock::unlock() {
	int error;
	if ((error = ::pthread_rwlock_unlock(rwlock)) != 0) {
		throw std::system_error(error, std::system_category(), "pthread_rwlock_unlock");
	}
}


void WriteLock::lock() {
	int error;
	if ((error = ::pthread_rwlock_wrlock(rwlock)) != 0) {
		throw std::system_error(error, std::system_category(), "pthread_rwlock_wrlock");
	}
}

bool WriteLock::try_lock() {
	int error;
	if ((error = ::pthread_rwlock_trywrlock(rwlock)) != 0) {
		if (error == EBUSY) {
			return false;
		}
		throw std::system_error(error, std::system_category(), "pthread_rwlock_trywrlock");
	}
	return true;
}

bool WriteLock::try_lock_for(const timespec &timeout_duration) {
	int error;
	if ((error = ::pthread_rwlock_timedwrlock(rwlock, &timeout_duration)) != 0) {
		if (error == ETIMEDOUT) {
			return false;
		}
		throw std::system_error(error, std::system_category(), "pthread_rwlock_timedwrlock");
	}
	return true;
}

void WriteLock::unlock() {
	int error;
	if ((error = ::pthread_rwlock_unlock(rwlock)) != 0) {
		throw std::system_error(error, std::system_category(), "pthread_rwlock_unlock");
	}
}


RWLock::RWLock() : rwlock() {
	int error;
	if ((error = ::pthread_rwlock_init(&rwlock, nullptr)) != 0) {
		throw std::system_error(error, std::system_category(), "pthread_rwlock_init");
	}
}

RWLock::~RWLock() {
	int error;
	if ((error = ::pthread_rwlock_destroy(&rwlock)) != 0) {
		throw std::system_error(error, std::system_category(), "pthread_rwlock_destroy");
	}
}


} // namespace posix
