#include "linux_eventfd.h"

#include <system_error>


namespace linux {


EventFD::EventFD(unsigned int initval, int flags) : FileDescriptor(::eventfd(initval, flags)) {
	if (_unlikely(fd < 0)) {
		throw std::system_error(errno, std::system_category(), "eventfd");
	}
}

eventfd_t EventFD::read() {
	eventfd_t ret;
	if (::eventfd_read(fd, &ret) < 0) {
		if (_unlikely(errno != EAGAIN)) {
			throw std::system_error(errno, std::system_category(), "eventfd_read");
		}
		return 0;
	}
	return ret;
}

bool EventFD::write(eventfd_t value) {
	if (::eventfd_write(fd, value) < 0) {
		if (_unlikely(errno != EAGAIN)) {
			throw std::system_error(errno, std::system_category(), "eventfd_write");
		}
		return false;
	}
	return true;
}


} // namespace linux
