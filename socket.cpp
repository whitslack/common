#include "socket.h"

#include <cassert>
#include <system_error>

#include <netinet/tcp.h>
#include <sys/ioctl.h>


namespace posix {

int accept(int socket, struct sockaddr * _restrict address, socklen_t * _restrict address_len) {
	int ret;
	if (_unlikely((ret = ::accept(socket, address, address_len)) < 0)) {
		throw std::system_error(errno, std::system_category(), "accept");
	}
	return ret;
}

int accept(int socket, struct sockaddr * _restrict address, socklen_t * _restrict address_len, int flags) {
	int ret;
	if (_unlikely((ret = ::accept4(socket, address, address_len, flags)) < 0)) {
		throw std::system_error(errno, std::system_category(), "accept4");
	}
	return ret;
}

void bind(int socket, const struct sockaddr *address, socklen_t address_len) {
	if (_unlikely(::bind(socket, address, address_len) < 0)) {
		throw std::system_error(errno, std::system_category(), "bind");
	}
}

bool connect(int socket, const struct sockaddr *address, socklen_t address_len) {
	if (::connect(socket, address, address_len) < 0) {
		if (_unlikely(errno != EINPROGRESS)) {
			throw std::system_error(errno, std::system_category(), "connect");
		}
		return false;
	}
	return true;
}

void getpeername(int socket, struct sockaddr * _restrict address, socklen_t * _restrict address_len) {
	if (_unlikely(::getpeername(socket, address, address_len) < 0)) {
		throw std::system_error(errno, std::system_category(), "getpeername");
	}
}

void getsockname(int socket, struct sockaddr * _restrict address, socklen_t * _restrict address_len) {
	if (_unlikely(::getsockname(socket, address, address_len) < 0)) {
		throw std::system_error(errno, std::system_category(), "getsockname");
	}
}

void getsockopt(int socket, int level, int option_name, void * _restrict option_value, socklen_t * _restrict option_len) {
	if (_unlikely(::getsockopt(socket, level, option_name, option_value, option_len) < 0)) {
		throw std::system_error(errno, std::system_category(), "getsockopt");
	}
}

void listen(int socket, int backlog) {
	if (_unlikely(::listen(socket, backlog) < 0)) {
		throw std::system_error(errno, std::system_category(), "listen");
	}
}

ssize_t recv(int socket, void *buffer, size_t length, int flags) {
	ssize_t ret;
	if ((ret = ::recv(socket, buffer, length, flags)) < 0) {
		if (_unlikely(errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR)) {
			throw std::system_error(errno, std::system_category(), "recv");
		}
		return 0;
	}
	return ret == 0 && length > 0 ? -1 : ret;
}

ssize_t recvfrom(int socket, void * _restrict buffer, size_t length, int flags, struct sockaddr * _restrict address, socklen_t * _restrict address_len) {
	ssize_t ret;
	if ((ret = ::recvfrom(socket, buffer, length, flags, address, address_len)) < 0) {
		if (_unlikely(errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR)) {
			throw std::system_error(errno, std::system_category(), "recvfrom");
		}
		return 0;
	}
	return ret == 0 && length > 0 ? -1 : ret;
}

ssize_t recvmsg(int socket, struct msghdr *message, int flags) {
	ssize_t ret;
	if ((ret = ::recvmsg(socket, message, flags)) < 0) {
		if (_unlikely(errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR)) {
			throw std::system_error(errno, std::system_category(), "recvmsg");
		}
		return 0;
	}
	return ret == 0 ? -1 : ret;
}

size_t send(int socket, const void *buffer, size_t length, int flags) {
	ssize_t ret;
	if ((ret = ::send(socket, buffer, length, flags)) < 0) {
		if (_unlikely(errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR)) {
			throw std::system_error(errno, std::system_category(), "send");
		}
		return 0;
	}
	return static_cast<size_t>(ret);
}

size_t sendmsg(int socket, const struct msghdr *message, int flags) {
	ssize_t ret;
	if ((ret = ::sendmsg(socket, message, flags)) < 0) {
		if (_unlikely(errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR)) {
			throw std::system_error(errno, std::system_category(), "sendmsg");
		}
		return 0;
	}
	return static_cast<size_t>(ret);
}

size_t sendto(int socket, const void *message, size_t length, int flags, const struct sockaddr *dest_addr, socklen_t dest_len) {
	ssize_t ret;
	if ((ret = ::sendto(socket, message, length, flags, dest_addr, dest_len)) < 0) {
		if (_unlikely(errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR)) {
			throw std::system_error(errno, std::system_category(), "sendto");
		}
		return 0;
	}
	return static_cast<size_t>(ret);
}

void setsockopt(int socket, int level, int option_name, const void *option_value, socklen_t option_len) {
	if (_unlikely(::setsockopt(socket, level, option_name, option_value, option_len) < 0)) {
		throw std::system_error(errno, std::system_category(), "setsockopt");
	}
}

void shutdown(int socket, int how) {
	if (_unlikely(::shutdown(socket, how) < 0)) {
		throw std::system_error(errno, std::system_category(), "shutdown");
	}
}

bool sockatmark(int s) {
	int ret;
	if (_unlikely((ret = ::sockatmark(s)) < 0)) {
		throw std::system_error(errno, std::system_category(), "sockatmark");
	}
	return ret != 0;
}

int socket(int domain, int type, int protocol) {
	int ret;
	if (_unlikely((ret = ::socket(domain, type, protocol)) < 0)) {
		throw std::system_error(errno, std::system_category(), "socket");
	}
	return ret;
}

void socketpair(int domain, int type, int protocol, int socket_vector[2]) {
	if (_unlikely(::socketpair(domain, type, protocol, socket_vector) < 0)) {
		throw std::system_error(errno, std::system_category(), "socketpair");
	}
}

} // namespace posix


std::chrono::microseconds Socket::recv_timeout() const {
	struct timeval tv { };
	socklen_t size = sizeof tv;
	this->getsockopt(SOL_SOCKET, SO_RCVTIMEO, &tv, &size);
	assert(size == sizeof tv);
	return posix::timeval_to_duration(tv);
}

std::chrono::microseconds Socket::send_timeout() const {
	struct timeval tv { };
	socklen_t size = sizeof tv;
	this->getsockopt(SOL_SOCKET, SO_SNDTIMEO, &tv, &size);
	assert(size == sizeof tv);
	return posix::timeval_to_duration(tv);
}

SocketAddress Socket::getsockname() const {
	struct sockaddr_storage ss;
	socklen_t size { sizeof ss };
	this->getsockname(reinterpret_cast<struct sockaddr *>(&ss), &size);
	if (_likely(size <= sizeof ss)) {
		return { reinterpret_cast<const struct sockaddr *>(&ss), size };
	}
	SocketAddress ret;
	this->getsockname(ret.alloc(size), &size);
	assert(size == ret.size());
	return ret;
}

SocketAddress Socket::getpeername() const {
	struct sockaddr_storage ss;
	socklen_t size { sizeof ss };
	this->getsockname(reinterpret_cast<struct sockaddr *>(&ss), &size);
	if (_likely(size <= sizeof ss)) {
		return { reinterpret_cast<const struct sockaddr *>(&ss), size };
	}
	SocketAddress ret;
	this->getsockname(ret.alloc(size), &size);
	assert(size == ret.size());
	return ret;
}

bool Socket::connect(const struct sockaddr *address, socklen_t address_len) {
	bool ret = posix::connect(fd, address, address_len);
#ifdef TCP_CORK
	this->setsockopt(IPPROTO_TCP, TCP_NODELAY, 1);
	this->setsockopt(IPPROTO_TCP, TCP_CORK, 1);
#elif defined(TCP_NOPUSH)
	this->setsockopt(IPPROTO_TCP, TCP_NODELAY, 1);
	this->setsockopt(IPPROTO_TCP, TCP_NOPUSH, 1);
#endif
	return ret;
}

Socket Socket::accept(struct sockaddr * _restrict address, socklen_t * _restrict address_len, int flags) {
	Socket socket(FileDescriptor(posix::accept(fd, address, address_len, flags)));
#ifdef TCP_CORK
	socket.setsockopt(IPPROTO_TCP, TCP_NODELAY, 1);
	socket.setsockopt(IPPROTO_TCP, TCP_CORK, 1);
#elif defined(TCP_NOPUSH)
	socket.setsockopt(IPPROTO_TCP, TCP_NODELAY, 1);
	socket.setsockopt(IPPROTO_TCP, TCP_NOPUSH, 1);
#endif
	return socket;
}

Socket Socket::accept(SocketAddress &address, int flags) {
	struct sockaddr_storage ss;
	socklen_t size { sizeof ss };
	auto ret = this->accept(reinterpret_cast<struct sockaddr *>(&ss), &size, flags);
	assert(size <= sizeof ss);
	address.assign(reinterpret_cast<const struct sockaddr *>(&ss), std::min<socklen_t>(size, sizeof ss));
	return ret;
}

ssize_t Socket::recvfrom(void * _restrict buffer, size_t length, int flags, SocketAddress &address) {
	struct sockaddr_storage ss;
	socklen_t size { sizeof ss };
	auto ret = this->recvfrom(buffer, length, flags, reinterpret_cast<struct sockaddr *>(&ss), &size);
	assert(size <= sizeof ss);
	address.assign(reinterpret_cast<const struct sockaddr *>(&ss), std::min<socklen_t>(size, sizeof ss));
	return ret;
}

bool Socket::flush() {
#ifdef TCP_CORK
# ifdef __linux__
	// On Linux, setting TCP_NODELAY, even if it's already set, triggers an explicit push.
	this->setsockopt(IPPROTO_TCP, TCP_NODELAY, 1);
# else
	this->setsockopt(IPPROTO_TCP, TCP_CORK, 0);
	this->setsockopt(IPPROTO_TCP, TCP_CORK, 1);
# endif
#elif defined(TCP_NOPUSH)
	this->setsockopt(IPPROTO_TCP, TCP_NOPUSH, 0);
	this->send(nullptr, 0, 0);
	this->setsockopt(IPPROTO_TCP, TCP_NOPUSH, 1);
#endif
	return true;
}

template class Flushable<Socket>;


#if __NEED_SOCKET_POLYFILL

static int set_socket_flags(int fd, int flags) noexcept {
	if (flags & SOCK_CLOEXEC) {
		int f = ::fcntl(fd, F_GETFD);
		if (f < 0 || ::fcntl(fd, F_SETFD, f | FD_CLOEXEC) < 0) {
			return -1;
		}
	}
	if (flags & SOCK_NONBLOCK) {
		int f = ::fcntl(fd, F_GETFL);
		if (f < 0 || ::fcntl(fd, F_SETFL, f | O_NONBLOCK) < 0) {
			return -1;
		}
	}
	return 0;
}

int __accept4_polyfill(int socket, struct sockaddr * _restrict address, socklen_t * _restrict address_len, int flags) noexcept {
	if (flags & ~(SOCK_NONBLOCK | SOCK_CLOEXEC)) {
		errno = EINVAL;
		return -1;
	}
	int fd = ::accept(socket, address, address_len);
	if (fd >= 0 && set_socket_flags(fd, flags) < 0) {
		::close(fd);
		return -1;
	}
	return fd;
}

int __socket_polyfill(int domain, int type, int protocol) noexcept {
	int fd = ::__socket(domain, type & ~(SOCK_NONBLOCK | SOCK_CLOEXEC), protocol);
	if (fd >= 0 && set_socket_flags(fd, type) < 0) {
		::close(fd);
		return -1;
	}
	return fd;
}

#endif // __NEED_SOCKET_POLYFILL
