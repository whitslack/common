#pragma once

#include <iosfwd>

#include <netinet/in.h>

#include "fd.h"


namespace posix {

int accept(int socket, struct sockaddr * _restrict address, socklen_t * _restrict address_len);
int accept(int socket, struct sockaddr * _restrict address, socklen_t * _restrict address_len, int flags);
void bind(int socket, const struct sockaddr *address, socklen_t address_len);
bool connect(int socket, const struct sockaddr *address, socklen_t address_len);
void getpeername(int socket, struct sockaddr * _restrict address, socklen_t * _restrict address_len);
void getsockname(int socket, struct sockaddr * _restrict address, socklen_t * _restrict address_len);
void getsockopt(int socket, int level, int option_name, void * _restrict option_value, socklen_t * _restrict option_len);
void listen(int socket, int backlog = 0);
ssize_t recv(int socket, void *buffer, size_t length, int flags = 0);
ssize_t recvfrom(int socket, void * _restrict buffer, size_t length, int flags, struct sockaddr * _restrict address, socklen_t * _restrict address_len);
ssize_t recvmsg(int socket, struct msghdr *message, int flags = 0);
size_t send(int socket, const void *buffer, size_t length, int flags = 0);
size_t sendmsg(int socket, const struct msghdr *message, int flags = 0);
size_t sendto(int socket, const void *message, size_t length, int flags, const struct sockaddr *dest_addr, socklen_t dest_len);
void setsockopt(int socket, int level, int option_name, const void *option_value, socklen_t option_len);
void shutdown(int socket, int how = SHUT_RDWR);
bool sockatmark(int s);
int socket(int domain, int type, int protocol);
void socketpair(int domain, int type, int protocol, int socket_vector[2]);

} // namespace posix


class Socket : public FileDescriptor {

public:
	Socket() { }
	Socket(int domain, int type, int protocol = 0) : FileDescriptor(posix::socket(domain, type, protocol)) { }

protected:
	explicit Socket(int fd) : FileDescriptor(fd) { }

public:
	void open(int domain, int type, int protocol = 0) { *this = Socket(domain, type, protocol); }
	void shutdown(int how = SHUT_RDWR) { posix::shutdown(fd, how); }

	void getsockopt(int level, int optname, void *optval, socklen_t *optlen) const { posix::getsockopt(fd, level, optname, optval, optlen); }
	void setsockopt(int level, int optname, const void *optval, socklen_t optlen) { posix::setsockopt(fd, level, optname, optval, optlen); }

	void getsockname(sockaddr *addr, socklen_t *addrlen) const { posix::getsockname(fd, addr, addrlen); }
	void bind(const sockaddr *addr, socklen_t addrlen) { posix::bind(fd, addr, addrlen); }

	void getpeername(sockaddr *addr, socklen_t *addrlen) const { posix::getpeername(fd, addr, addrlen); }
	bool connect(const sockaddr *addr, socklen_t addrlen) { return posix::connect(fd, addr, addrlen); }
	Socket accept(sockaddr *addr, socklen_t *addrlen, int flags = SOCK_NONBLOCK | SOCK_CLOEXEC) { return Socket(posix::accept(fd, addr, addrlen, flags)); }

	void listen(int backlog = SOMAXCONN) { posix::listen(fd, backlog); }
	ssize_t recv(void *buffer, size_t length, int flags = 0) { return posix::recv(fd, buffer, length, flags); }
	size_t send(const void *, size_t, bool) = delete;
	size_t send(const void *buffer, size_t length, int flags = MSG_NOSIGNAL) { return posix::send(fd, buffer, length, flags); }

	size_t avail() override;
	size_t write(const void *buf, size_t n) override;
	bool flush() override;

};


template <typename T, typename A>
class SocketBase : public Socket {

protected:
	using Socket::Socket;

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
	explicit Socket4(int type, int protocol = 0, int flags = SOCK_NONBLOCK | SOCK_CLOEXEC) : SocketBase(AF_INET, type | flags, protocol) { }

protected:
	Socket4(int fd, std::nullptr_t) : SocketBase<Socket4, sockaddr_in>(fd) { }

public:
	void open(int type = SOCK_STREAM, int protocol = 0, int flags = SOCK_NONBLOCK | SOCK_CLOEXEC) { *this = Socket4(type, protocol, flags); }

};


class Socket6 : public SocketBase<Socket6, sockaddr_in6> {
	friend SocketBase<Socket6, sockaddr_in6>;

public:
	Socket6() { }
	explicit Socket6(int type, int protocol = 0, int flags = SOCK_NONBLOCK | SOCK_CLOEXEC) : SocketBase(AF_INET6, type | flags, protocol) { }

protected:
	Socket6(int fd, std::nullptr_t) : SocketBase<Socket6, sockaddr_in6>(fd) { }

public:
	void open(int type = SOCK_STREAM, int protocol = 0, int flags = SOCK_NONBLOCK | SOCK_CLOEXEC) { *this = Socket6(type, protocol, flags); }

};


std::ostream & operator << (std::ostream &os, const in_addr &addr);
std::ostream & operator << (std::ostream &os, const in6_addr &addr);
