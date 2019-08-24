#pragma once

#include <chrono>

#include <netinet/in.h>
#include <sys/socket.h>

#include "fd.h"
#include "selector.h"
#include "sockaddr.h"


namespace posix {

_nodiscard int accept(int socket, struct sockaddr * _restrict address, socklen_t * _restrict address_len);
_nodiscard int accept(int socket, struct sockaddr * _restrict address, socklen_t * _restrict address_len, int flags);
void bind(int socket, const struct sockaddr *address, socklen_t address_len);
bool connect(int socket, const struct sockaddr *address, socklen_t address_len);
void getpeername(int socket, struct sockaddr * _restrict address, socklen_t * _restrict address_len);
void getsockname(int socket, struct sockaddr * _restrict address, socklen_t * _restrict address_len);
void getsockopt(int socket, int level, int option_name, void * _restrict option_value, socklen_t * _restrict option_len);
void listen(int socket, int backlog = SOMAXCONN);
_nodiscard ssize_t recv(int socket, void *buffer, size_t length, int flags);
_nodiscard ssize_t recvfrom(int socket, void * _restrict buffer, size_t length, int flags, struct sockaddr * _restrict address, socklen_t * _restrict address_len);
_nodiscard ssize_t recvmsg(int socket, struct msghdr *message, int flags = 0);
_nodiscard size_t send(int socket, const void *buffer, size_t length, int flags);
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
	Socket() = default;
	explicit Socket(int domain, int type, int protocol = 0) : FileDescriptor(posix::socket(domain, type, protocol)) { }
	explicit Socket(FileDescriptor &&move) noexcept : FileDescriptor(std::move(move)) { }

public:
	void open(int domain, int type, int protocol = 0) { *this = Socket(domain, type, protocol); }
	void shutdown(int how = SHUT_RDWR) { posix::shutdown(fd, how); }

	void getsockopt(int level, int option_name, void * _restrict option_value, socklen_t * _restrict option_len) const { posix::getsockopt(fd, level, option_name, option_value, option_len); }
	void setsockopt(int level, int option_name, const void *option_value, socklen_t option_len) { posix::setsockopt(fd, level, option_name, option_value, option_len); }
	template <typename T> void setsockopt(int level, int option_name, const T &option_value) { this->setsockopt(level, option_name, &option_value, static_cast<socklen_t>(sizeof option_value)); }

	std::chrono::microseconds _pure recv_timeout() const;
	void recv_timeout(std::chrono::microseconds timeout) { return this->setsockopt(SOL_SOCKET, SO_RCVTIMEO, posix::duration_to_timeval(timeout)); }
	std::chrono::microseconds _pure send_timeout() const;
	void send_timeout(std::chrono::microseconds timeout) { return this->setsockopt(SOL_SOCKET, SO_SNDTIMEO, posix::duration_to_timeval(timeout)); }

	void getsockname(struct sockaddr * _restrict address, socklen_t * _restrict address_len) const { posix::getsockname(fd, address, address_len); }
	SocketAddress _pure getsockname() const;

	void bind(const struct sockaddr *address, socklen_t address_len) { posix::bind(fd, address, address_len); }
	void bind(const struct sockaddr_in &address) { return this->bind(reinterpret_cast<const struct sockaddr *>(&address), socklen_t { sizeof address }); }
	void bind(const struct sockaddr_in6 &address) { return this->bind(reinterpret_cast<const struct sockaddr *>(&address), socklen_t { sizeof address }); }
	void bind(const SocketAddress &address) { return this->bind(address, address.size()); }

	void getpeername(struct sockaddr * _restrict address, socklen_t * _restrict address_len) const { posix::getpeername(fd, address, address_len); }
	SocketAddress _pure getpeername() const;

	bool connect(const struct sockaddr *address, socklen_t address_len);
	bool connect(const struct sockaddr_in &address) { return this->connect(reinterpret_cast<const struct sockaddr *>(&address), socklen_t { sizeof address }); }
	bool connect(const struct sockaddr_in6 &address) { return this->connect(reinterpret_cast<const struct sockaddr *>(&address), socklen_t { sizeof address }); }
	bool connect(const SocketAddress &address) { return this->connect(address, address.size()); }

	Socket accept(struct sockaddr * _restrict address = nullptr, socklen_t * _restrict address_len = nullptr, int flags = SOCK_CLOEXEC);
	Socket accept(SocketAddress &address, int flags = SOCK_CLOEXEC);

	void listen(int backlog = SOMAXCONN) { posix::listen(fd, backlog); }
	_nodiscard ssize_t recv(void *buffer, size_t length, int flags) { return posix::recv(fd, buffer, length, flags); }
	_nodiscard ssize_t recvfrom(void * _restrict buffer, size_t length, int flags, SocketAddress &address);
	_nodiscard ssize_t recvfrom(void * _restrict buffer, size_t length, int flags, struct sockaddr * _restrict address, socklen_t * _restrict address_len) { return posix::recvfrom(fd, buffer, length, flags, address, address_len); }
	_nodiscard ssize_t recvmsg(struct msghdr *message, int flags = 0) { return posix::recvmsg(fd, message, flags); }
	_nodiscard size_t send(const void *, size_t, bool) = delete;
	_nodiscard size_t send(const void *buffer, size_t length, int flags) { return posix::send(fd, buffer, length, flags); }
	_nodiscard size_t sendmsg(const struct msghdr *message, int flags = 0) { return posix::sendmsg(fd, message, flags); }
	_nodiscard size_t sendto(const void *message, size_t length, int flags, const SocketAddress &dest_addr) { return this->sendto(message, length, flags, dest_addr, dest_addr.size()); }
	_nodiscard size_t sendto(const void *message, size_t length, int flags, const struct sockaddr *dest_addr, socklen_t dest_len) { return posix::sendto(fd, message, length, flags, dest_addr, dest_len); }

	bool flush() override;

};


#if __NEED_SOCKET_POLYFILL
extern "C" {
	_nodiscard int __accept4_polyfill(int socket, struct sockaddr * _restrict address, socklen_t * _restrict address_len, int flags) noexcept;
	_nodiscard extern inline _always_inline int accept4(int socket, struct sockaddr * _restrict address, socklen_t * _restrict address_len, int flags) noexcept {
		return __builtin_constant_p(flags) && flags == 0 ? ::accept(socket, address, address_len) : ::__accept4_polyfill(socket, address, address_len, flags);
	}
	_nodiscard int __socket(int domain, int type, int protocol) __asm__ (__USER_LABEL_PREFIX_STR__ "socket");
	_nodiscard int __socket_polyfill(int domain, int type, int protocol) noexcept;
	_nodiscard extern inline _always_inline int socket(int domain, int type, int protocol) noexcept {
		return __builtin_constant_p(type) && !(type & (SOCK_NONBLOCK | SOCK_CLOEXEC)) ? ::__socket(domain, type, protocol) : ::__socket_polyfill(domain, type, protocol);
	}
}
#endif // __NEED_SOCKET_POLYFILL
