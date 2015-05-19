#include "eventfd.h"

#include <system_error>


EventFD::EventFD(unsigned int initval, int flags) : FileDescriptor(::eventfd(initval, flags)) {
	if (fd < 0) {
		throw std::system_error(errno, std::system_category(), "eventfd");
	}
}

eventfd_t EventFD::read() {
	eventfd_t ret;
	if (::eventfd_read(fd, &ret) < 0) {
		throw std::system_error(errno, std::system_category(), "eventfd_read");
	}
	return ret;
}

void EventFD::write(eventfd_t value) {
	if (::eventfd_write(fd, value) < 0) {
		throw std::system_error(errno, std::system_category(), "eventfd_write");
	}
}
