#pragma once

#include <chrono>

#include "enumflags.h"
#include "fd.h"
#include "signal.h"


class Selectable;


class Selector {

public:
	enum class Flags {
		NONE = 0,
		READABLE = 1 << 0,
		WRITABLE = 1 << 1,
	};

private:
#ifdef __linux__
	FileDescriptor epoll_fd, event_fd;
#endif

public:
	Selector();

public:
	void add(FileDescriptor &fd, void *ptr, Flags flags);
	void add(FileDescriptor &fd, Selectable *ptr, Flags flags) { return this->add(fd, static_cast<void *>(ptr), flags); }
	void modify(FileDescriptor &fd, void *ptr, Flags flags);
	void modify(FileDescriptor &fd, Selectable *ptr, Flags flags) { return this->modify(fd, static_cast<void *>(ptr), flags); }
	void remove(FileDescriptor &fd);

	std::pair<void *, Flags> select();
	std::pair<void *, Flags> select(std::chrono::milliseconds timeout);

	std::pair<void *, Flags> pselect(const sigset_t *sigmask);
	std::pair<void *, Flags> pselect(std::chrono::milliseconds timeout, const sigset_t *sigmask);

	void kick();

};
DEFINE_ENUM_FLAG_OPS(Selector::Flags)


class Selectable {

public:
	static void _noreturn pump(Selector &selector);

public:
	virtual ~Selectable() { }

protected:
	virtual void selected(Selector &selector, Selector::Flags flags) noexcept = 0;

};
