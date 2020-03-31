#include "selector.h"


#ifdef __linux__

#include <climits>

#include <sys/epoll.h>
#include <sys/eventfd.h>

#undef linux
namespace linux {

static void epoll_ctl(FileDescriptor &epoll_fd, int op, FileDescriptor &fd, void *ptr, Selector::Flags flags) {
	struct epoll_event event;
	event.events = EPOLLONESHOT |
			((flags & Selector::Flags::READABLE) != Selector::Flags::NONE ? static_cast<uint32_t>(EPOLLIN) : 0) |
			((flags & Selector::Flags::WRITABLE) != Selector::Flags::NONE ? static_cast<uint32_t>(EPOLLOUT) : 0);
	event.data.ptr = ptr;
	if (_unlikely(::epoll_ctl(epoll_fd, op, fd, &event) < 0)) {
		throw std::system_error(errno, std::system_category(), "epoll_ctl");
	}
}

static std::pair<void *, Selector::Flags> epoll_wait(FileDescriptor &epoll_fd, int timeout) {
	struct epoll_event event;
	int n = ::epoll_wait(epoll_fd, &event, 1, timeout);
	if (_unlikely(n < 0 && errno != EINTR)) {
		throw std::system_error(errno, std::system_category(), "epoll_wait");
	}
	if (n <= 0) {
		return { nullptr, Selector::Flags::NONE };
	}
	return { +event.data.ptr,
			(event.events & EPOLLIN ? Selector::Flags::READABLE : Selector::Flags::NONE) |
			(event.events & EPOLLOUT ? Selector::Flags::WRITABLE : Selector::Flags::NONE) };
}

static std::pair<void *, Selector::Flags> epoll_pwait(FileDescriptor &epoll_fd, int timeout, const sigset_t *sigmask) {
	struct epoll_event event;
	int n = ::epoll_pwait(epoll_fd, &event, 1, timeout, sigmask);
	if (_unlikely(n < 0 && errno != EINTR)) {
		throw std::system_error(errno, std::system_category(), "epoll_pwait");
	}
	if (n <= 0) {
		return { nullptr, Selector::Flags::NONE };
	}
	return { +event.data.ptr,
			(event.events & EPOLLIN ? Selector::Flags::READABLE : Selector::Flags::NONE) |
			(event.events & EPOLLOUT ? Selector::Flags::WRITABLE : Selector::Flags::NONE) };
}

} // namespace linux

Selector::Selector() : epoll_fd(::epoll_create1(EPOLL_CLOEXEC)) {
	if (_unlikely(!epoll_fd)) {
		throw std::system_error(errno, std::system_category(), "epoll_create1");
	}
}

void Selector::add(FileDescriptor &fd, void *ptr, Flags flags) {
	return linux::epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, ptr, flags);
}

void Selector::modify(FileDescriptor &fd, void *ptr, Flags flags) {
	return linux::epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, ptr, flags);
}

void Selector::remove(FileDescriptor &fd) {
	return linux::epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, nullptr, Flags::NONE);
}

std::pair<void *, Selector::Flags> Selector::select() {
	return linux::epoll_wait(epoll_fd, -1);
}

std::pair<void *, Selector::Flags> Selector::select(std::chrono::milliseconds timeout) {
	return linux::epoll_wait(epoll_fd, static_cast<int>(std::min(std::max(timeout, std::chrono::milliseconds::zero()), std::chrono::milliseconds(INT_MAX)).count()));
}

std::pair<void *, Selector::Flags> Selector::pselect(const sigset_t *sigmask) {
	return linux::epoll_pwait(epoll_fd, -1, sigmask);
}

std::pair<void *, Selector::Flags> Selector::pselect(std::chrono::milliseconds timeout, const sigset_t *sigmask) {
	return linux::epoll_pwait(epoll_fd, static_cast<int>(std::min(std::max(timeout, std::chrono::milliseconds::zero()), std::chrono::milliseconds(INT_MAX)).count()), sigmask);
}

void Selector::kick() {
	struct epoll_event event;
	event.events = EPOLLIN | EPOLLONESHOT;
	event.data.ptr = nullptr;
	int op;
	if (!event_fd) {
		if (_unlikely(!(event_fd = FileDescriptor(::eventfd(1, EFD_CLOEXEC))))) {
			throw std::system_error(errno, std::system_category(), "eventfd");
		}
		op = EPOLL_CTL_ADD;
	}
	else {
		op = EPOLL_CTL_MOD;
	}
	if (_unlikely(::epoll_ctl(epoll_fd, op, event_fd, &event) < 0)) {
		throw std::system_error(errno, std::system_category(), "epoll_ctl");
	}
}

#endif // defined(__linux__)


void Selectable::pump(Selector &selector) {
	for (;;) {
		auto pair = selector.select();
		if (pair.first) {
			static_cast<Selectable *>(pair.first)->selected(selector, pair.second);
		}
	}
}
