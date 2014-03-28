#pragma once

#include <netinet/in.h>

#include "fd.h"


class Socket : public FileDescriptor {

public:
	Socket() { }
	Socket(int domain, int type, int protocol = 0) { this->open(domain, type, protocol); }

protected:
	explicit Socket(int fd) : FileDescriptor(fd) { }

public:
	void open(int domain, int type, int protocol = 0);
	void shutdown(int how = SHUT_RDWR);

	void getsockopt(int level, int optname, void *optval, socklen_t *optlen) const;
	void setsockopt(int level, int optname, const void *optval, socklen_t optlen);

	void getsockname(sockaddr *addr, socklen_t *addrlen) const;
	void bind(const sockaddr *addr, socklen_t addrlen);

	void getpeername(sockaddr *addr, socklen_t *addrlen) const;
	bool connect(const sockaddr *addr, socklen_t addrlen);
	Socket accept(sockaddr *addr, socklen_t *addrlen, int flags = SOCK_NONBLOCK | SOCK_CLOEXEC);

	void listen(int backlog = SOMAXCONN);
	ssize_t recv(void *buf, size_t n, int flags = 0);
	size_t send(const void *, size_t, bool) = delete;
	size_t send(const void *buf, size_t n, int flags = MSG_NOSIGNAL);

	size_t avail() override;
	size_t write(const void *buf, size_t n) override;
	bool flush() override;

};


template <typename T, typename A>
class SocketBase : public Socket {

protected:
	SocketBase() { }
	explicit SocketBase(int fd) : Socket(fd) { }

public:
	void getsockname(A &addr) const;
	void bind(const A &addr);

	void getpeername(A &addr) const;
	bool connect(const A &addr);
	T accept(A *addr = nullptr, int flags = SOCK_NONBLOCK | SOCK_CLOEXEC);

};


class Socket4 : public SocketBase<Socket4, sockaddr_in> {
	friend SocketBase<Socket4, sockaddr_in>;

public:
	Socket4() { }
	explicit Socket4(int type, int protocol = 0, int flags = SOCK_NONBLOCK | SOCK_CLOEXEC) { this->open(type, protocol, flags); }

protected:
	Socket4(int fd, std::nullptr_t) : SocketBase<Socket4, sockaddr_in>(fd) { }

public:
	void open(int type = SOCK_STREAM, int protocol = 0, int flags = SOCK_NONBLOCK | SOCK_CLOEXEC) { this->SocketBase::open(AF_INET, type | flags, protocol); }

};


class Socket6 : public SocketBase<Socket6, sockaddr_in6> {
	friend SocketBase<Socket6, sockaddr_in6>;

public:
	Socket6() { }
	explicit Socket6(int type, int protocol = 0, int flags = SOCK_NONBLOCK | SOCK_CLOEXEC) { this->open(type, protocol, flags); }

protected:
	Socket6(int fd, std::nullptr_t) : SocketBase<Socket6, sockaddr_in6>(fd) { }

public:
	void open(int type = SOCK_STREAM, int protocol = 0, int flags = SOCK_NONBLOCK | SOCK_CLOEXEC) { this->SocketBase::open(AF_INET6, type | flags, protocol); }

};
