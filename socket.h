#pragma once

#include <iosfwd>

#include <netinet/in.h>

#include "fd.h"


namespace posix {

_nodiscard int accept(int socket, struct sockaddr * _restrict address, socklen_t * _restrict address_len);
_nodiscard int accept(int socket, struct sockaddr * _restrict address, socklen_t * _restrict address_len, int flags);
void bind(int socket, const struct sockaddr *address, socklen_t address_len);
bool connect(int socket, const struct sockaddr *address, socklen_t address_len);
void getpeername(int socket, struct sockaddr * _restrict address, socklen_t * _restrict address_len);
void getsockname(int socket, struct sockaddr * _restrict address, socklen_t * _restrict address_len);
void getsockopt(int socket, int level, int option_name, void * _restrict option_value, socklen_t * _restrict option_len);
void listen(int socket, int backlog = 0);
_nodiscard ssize_t recv(int socket, void *buffer, size_t length, int flags = 0);
_nodiscard ssize_t recvfrom(int socket, void * _restrict buffer, size_t length, int flags, struct sockaddr * _restrict address, socklen_t * _restrict address_len);
_nodiscard ssize_t recvmsg(int socket, struct msghdr *message, int flags = 0);
_nodiscard size_t send(int socket, const void *buffer, size_t length, int flags = 0);
_nodiscard size_t sendmsg(int socket, const struct msghdr *message, int flags = 0);
_nodiscard size_t sendto(int socket, const void *message, size_t length, int flags, const struct sockaddr *dest_addr, socklen_t dest_len);
void setsockopt(int socket, int level, int option_name, const void *option_value, socklen_t option_len);
void shutdown(int socket, int how = SHUT_RDWR);
bool sockatmark(int s);
_nodiscard int socket(int domain, int type, int protocol);
void socketpair(int domain, int type, int protocol, int socket_vector[2]);

} // namespace posix


#ifndef SOCK_NONBLOCK
#	define SOCK_NONBLOCK 0x800
#	define __NEED_SOCKET_POLYFILL 1
#endif
#ifndef SOCK_CLOEXEC
#	define SOCK_CLOEXEC 0x80000
#	undef __NEED_SOCKET_POLYFILL
#	define __NEED_SOCKET_POLYFILL 1
#endif


class Socket : public FileDescriptor {

public:
	Socket() { }
	Socket(int domain, int type, int protocol = 0) : FileDescriptor(posix::socket(domain, type, protocol)) { }
	explicit Socket(FileDescriptor &&move) : FileDescriptor(std::move(move)) { }

public:
	void open(int domain, int type, int protocol = 0) { *this = Socket(domain, type, protocol); }
	void shutdown(int how = SHUT_RDWR) { posix::shutdown(fd, how); }

	void getsockopt(int level, int optname, void *optval, socklen_t *optlen) const { posix::getsockopt(fd, level, optname, optval, optlen); }
	void setsockopt(int level, int optname, const void *optval, socklen_t optlen) { posix::setsockopt(fd, level, optname, optval, optlen); }
	template <typename T> void setsockopt(int level, int optname, const T &optval) { this->setsockopt(level, optname, &optval, static_cast<socklen_t>(sizeof optval)); }

	void getsockname(sockaddr *addr, socklen_t *addrlen) const { posix::getsockname(fd, addr, addrlen); }
	void bind(const sockaddr *addr, socklen_t addrlen) { posix::bind(fd, addr, addrlen); }

	void getpeername(sockaddr *addr, socklen_t *addrlen) const { posix::getpeername(fd, addr, addrlen); }
	bool connect(const sockaddr *addr, socklen_t addrlen);
	Socket accept(sockaddr *addr = nullptr, socklen_t *addrlen = nullptr, int flags = SOCK_CLOEXEC);

	void listen(int backlog = SOMAXCONN) { posix::listen(fd, backlog); }
	_nodiscard ssize_t recv(void *buffer, size_t length, int flags = 0) { return posix::recv(fd, buffer, length, flags); }
	_nodiscard size_t send(const void *, size_t, bool) = delete;
	_nodiscard size_t send(const void *buffer, size_t length, int flags = 0) { return posix::send(fd, buffer, length, flags); }

	size_t avail() override;
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
	T accept(A *addr = nullptr, int flags = SOCK_CLOEXEC);

};


class Socket4 : public SocketBase<Socket4, sockaddr_in> {

public:
	Socket4() { }
	explicit Socket4(int type, int protocol = 0, int flags = SOCK_CLOEXEC) : SocketBase(AF_INET, type | flags, protocol) { }
	explicit Socket4(FileDescriptor &&move) : SocketBase<Socket4, sockaddr_in>(std::move(move)) { }

public:
	void open(int type = SOCK_STREAM, int protocol = 0, int flags = SOCK_CLOEXEC) { *this = Socket4(type, protocol, flags); }

};


class Socket6 : public SocketBase<Socket6, sockaddr_in6> {

public:
	Socket6() { }
	explicit Socket6(int type, int protocol = 0, int flags = SOCK_CLOEXEC) : SocketBase(AF_INET6, type | flags, protocol) { }
	explicit Socket6(FileDescriptor &&move) : SocketBase<Socket6, sockaddr_in6>(std::move(move)) { }

public:
	void open(int type = SOCK_STREAM, int protocol = 0, int flags = SOCK_CLOEXEC) { *this = Socket6(type, protocol, flags); }

};


std::ostream & operator << (std::ostream &os, const in_addr &addr);
std::ostream & operator << (std::ostream &os, const in6_addr &addr);


#if __NEED_SOCKET_POLYFILL
extern "C" {
	_nodiscard int __accept4_polyfill(int socket, struct sockaddr * _restrict address, socklen_t * _restrict address_len, int flags);
	_nodiscard extern inline _always_inline int accept4(int socket, struct sockaddr * _restrict address, socklen_t * _restrict address_len, int flags) {
		return __builtin_constant_p(flags) && flags == 0 ? ::accept(socket, address, address_len) : ::__accept4_polyfill(socket, address, address_len, flags);
	}
	_nodiscard int __socket(int domain, int type, int protocol) __asm__ (__USER_LABEL_PREFIX_STR__ "socket");
	_nodiscard int __socket_polyfill(int domain, int type, int protocol);
	_nodiscard extern inline _always_inline int socket(int domain, int type, int protocol) {
		return __builtin_constant_p(type) && !(type & (SOCK_NONBLOCK | SOCK_CLOEXEC)) ? ::__socket(domain, type, protocol) : ::__socket_polyfill(domain, type, protocol);
	}
}
#endif // __NEED_SOCKET_POLYFILL
