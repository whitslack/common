#pragma once

#include <sys/epoll.h>

#include "fd.h"


class EPollable {
	friend class EPoll;

public:
	virtual ~EPollable() { }

protected:
	virtual operator int () const = 0;
	virtual void ready(class EPoll &epoll, uint32_t events) = 0;

};


class EPoll : public FileDescriptor {

public:
	EPoll();

	void pump(int timeout = -1);
	void watch(EPollable *epollable, uint32_t events, int op = EPOLL_CTL_MOD);

};
