#pragma once

#include <iosfwd>

#include <netinet/in.h>
#include <sys/socket.h>

#include "endian.h"
#include "fd.h"


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

	void getsockname(struct sockaddr * _restrict address, socklen_t * _restrict address_len) const { posix::getsockname(fd, address, address_len); }
	void bind(const struct sockaddr *address, socklen_t address_len) { posix::bind(fd, address, address_len); }

	void getpeername(struct sockaddr * _restrict address, socklen_t * _restrict address_len) const { posix::getpeername(fd, address, address_len); }
	bool connect(const struct sockaddr *address, socklen_t address_len);
	Socket accept(struct sockaddr * _restrict address = nullptr, socklen_t * _restrict address_len = nullptr, int flags = SOCK_CLOEXEC);

	void listen(int backlog = SOMAXCONN) { posix::listen(fd, backlog); }
	_nodiscard ssize_t recv(void *buffer, size_t length, int flags) { return posix::recv(fd, buffer, length, flags); }
	_nodiscard ssize_t recvfrom(void * _restrict buffer, size_t length, int flags, struct sockaddr * _restrict address, socklen_t * _restrict address_len) { return posix::recvfrom(fd, buffer, length, flags, address, address_len); }
	_nodiscard ssize_t recvmsg(struct msghdr *message, int flags = 0) { return posix::recvmsg(fd, message, flags); }
	_nodiscard size_t send(const void *, size_t, bool) = delete;
	_nodiscard size_t send(const void *buffer, size_t length, int flags) { return posix::send(fd, buffer, length, flags); }
	_nodiscard size_t sendmsg(const struct msghdr *message, int flags = 0) { return posix::sendmsg(fd, message, flags); }
	_nodiscard size_t sendto(const void *message, size_t length, int flags, const struct sockaddr *dest_addr, socklen_t dest_len) { return posix::sendto(fd, message, length, flags, dest_addr, dest_len); }

	size_t avail() override;
	bool flush() override;

};


template <typename T, typename A>
class SocketBase : public Socket {

protected:
	using Socket::Socket;

public:
	void getsockname(A &address) const;
	void bind(const A &address);

	void getpeername(A &address) const;
	bool connect(const A &address);
	T accept(A *address = nullptr, int flags = SOCK_CLOEXEC);

	_nodiscard ssize_t recvfrom(void * _restrict buffer, size_t length, int flags, A &address);
	_nodiscard size_t sendto(const void *message, size_t length, int flags, const A &address);

};


class Socket4 : public SocketBase<Socket4, struct sockaddr_in> {

public:
	Socket4() = default;
	explicit Socket4(int type, int protocol = 0, int flags = SOCK_CLOEXEC) : SocketBase(AF_INET, type | flags, protocol) { }
	explicit Socket4(FileDescriptor &&move) : SocketBase<Socket4, struct sockaddr_in>(std::move(move)) { }

public:
	void open(int type = SOCK_STREAM, int protocol = 0, int flags = SOCK_CLOEXEC) { *this = Socket4(type, protocol, flags); }

};


class Socket6 : public SocketBase<Socket6, struct sockaddr_in6> {

public:
	Socket6() = default;
	explicit Socket6(int type, int protocol = 0, int flags = SOCK_CLOEXEC) : SocketBase(AF_INET6, type | flags, protocol) { }
	explicit Socket6(FileDescriptor &&move) : SocketBase<Socket6, struct sockaddr_in6>(std::move(move)) { }

public:
	void open(int type = SOCK_STREAM, int protocol = 0, int flags = SOCK_CLOEXEC) { *this = Socket6(type, protocol, flags); }

};


static constexpr bool _pure operator == (const struct in_addr &addr1, const struct in_addr &addr2) noexcept {
	return std::memcmp(&addr1, &addr2, sizeof(struct in_addr)) == 0;
}

static constexpr bool _pure operator != (const struct in_addr &addr1, const struct in_addr &addr2) noexcept {
	return std::memcmp(&addr1, &addr2, sizeof(struct in_addr)) != 0;
}

static constexpr bool _pure operator == (const struct in6_addr &addr1, const struct in6_addr &addr2) noexcept {
	return std::memcmp(&addr1, &addr2, sizeof(struct in6_addr)) == 0;
}

static constexpr bool _pure operator != (const struct in6_addr &addr1, const struct in6_addr &addr2) noexcept {
	return std::memcmp(&addr1, &addr2, sizeof(struct in6_addr)) != 0;
}

static constexpr bool _pure operator == (const struct sockaddr_in &addr1, const struct sockaddr_in &addr2) noexcept {
	return std::memcmp(&addr1, &addr2, sizeof(struct sockaddr_in)) == 0;
}

static constexpr bool _pure operator != (const struct sockaddr_in &addr1, const struct sockaddr_in &addr2) noexcept {
	return std::memcmp(&addr1, &addr2, sizeof(struct sockaddr_in)) != 0;
}

static constexpr bool _pure operator == (const struct sockaddr_in6 &addr1, const struct sockaddr_in6 &addr2) noexcept {
	return std::memcmp(&addr1, &addr2, sizeof(struct sockaddr_in6)) == 0;
}

static constexpr bool _pure operator != (const struct sockaddr_in6 &addr1, const struct sockaddr_in6 &addr2) noexcept {
	return std::memcmp(&addr1, &addr2, sizeof(struct sockaddr_in6)) != 0;
}

template <>
struct std::hash<struct in_addr> {
	constexpr size_t _pure operator () (const struct in_addr &addr) const noexcept {
		return addr.s_addr;
	}
};

template <>
struct std::hash<struct in6_addr> {
	constexpr size_t _pure operator () (const struct in6_addr &addr) const noexcept {
		size_t hash { }, len = sizeof addr.s6_addr;
		for (auto ptr = reinterpret_cast<const std::byte *>(addr.s6_addr); len >= sizeof hash; ptr += sizeof hash, len -= sizeof hash) {
			hash ^= *reinterpret_cast<const size_t *>(ptr);
		}
		return as_be(hash);
	}
};

template <>
struct std::hash<struct sockaddr_in> {
	constexpr size_t _pure operator () (const struct sockaddr_in &addr) const noexcept {
		return hash<struct in_addr>()(addr.sin_addr) ^ as_be(addr.sin_port);
	}
};

template <>
struct std::hash<struct sockaddr_in6> {
	constexpr size_t _pure operator () (const struct sockaddr_in6 &addr) const noexcept {
		return hash<struct in6_addr>()(addr.sin6_addr) ^ as_be(addr.sin6_port);
	}
};

std::ostream & operator << (std::ostream &os, const struct in_addr &addr);
std::ostream & operator << (std::ostream &os, const struct in6_addr &addr);


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
