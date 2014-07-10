#include "epoll.h"

#include <system_error>


EPoll::EPoll() : FileDescriptor(::epoll_create1(EPOLL_CLOEXEC)) {
	if (fd < 0) {
		throw std::system_error(errno, std::system_category(), "epoll_create1");
	}
}

void EPoll::pump() {
	epoll_event event;
	int n;
	if ((n = ::epoll_wait(fd, &event, 1, -1)) < 0 && errno != EINTR) {
		throw std::system_error(errno, std::system_category(), "epoll_wait");
	}
	if (n > 0) {
		static_cast<EPollable *>(event.data.ptr)->ready(*this, event.events);
	}
}

void EPoll::watch(EPollable *epollable, uint32_t events, int op) {
	epoll_event event;
	event.events = events | EPOLLONESHOT;
	event.data.ptr = epollable;
	if (::epoll_ctl(fd, op, *epollable, &event) < 0) {
		throw std::system_error(errno, std::system_category(), "epoll_ctl");
	}
}
