#include "socket.h"

#include <cassert>

#include <netinet/tcp.h>


void Socket::open(int domain, int type, int protocol) {
	if (fd >= 0) {
		this->close();
	}
	if ((fd = ::socket(domain, type, protocol)) < 0) {
		throw std::system_error(errno, std::system_category(), "socket");
	}
}

void Socket::shutdown(int how) {
	if (::shutdown(fd, how) < 0) {
		throw std::system_error(errno, std::system_category(), "shutdown");
	}
}

void Socket::getsockopt(int level, int optname, void *optval, socklen_t *optlen) const {
	if (::getsockopt(fd, level, optname, optval, optlen) < 0) {
		throw std::system_error(errno, std::system_category(), "getsockopt");
	}
}

void Socket::setsockopt(int level, int optname, const void *optval, socklen_t optlen) {
	if (::setsockopt(fd, level, optname, optval, optlen) < 0) {
		throw std::system_error(errno, std::system_category(), "setsockopt");
	}
}

void Socket::getsockname(sockaddr *addr, socklen_t *addrlen) const {
	if (::getsockname(fd, addr, addrlen) < 0) {
		throw std::system_error(errno, std::system_category(), "getsockname");
	}
}

void Socket::bind(const sockaddr *addr, socklen_t addrlen) {
	if (::bind(fd, addr, addrlen) < 0) {
		throw std::system_error(errno, std::system_category(), "bind");
	}
}

void Socket::getpeername(sockaddr *addr, socklen_t *addrlen) const {
	if (::getpeername(fd, addr, addrlen) < 0) {
		throw std::system_error(errno, std::system_category(), "getpeername");
	}
}

bool Socket::connect(const sockaddr *addr, socklen_t addrlen) {
	if (::connect(fd, addr, addrlen) < 0) {
		if (errno == EINPROGRESS) {
			return false;
		}
		throw std::system_error(errno, std::system_category(), "connect");
	}
	return true;
}

Socket Socket::accept(sockaddr *addr, socklen_t *addrlen, int flags) {
	int s;
	if ((s = ::accept4(fd, addr, addrlen, flags)) < 0) {
		throw std::system_error(errno, std::system_category(), "accept4");
	}
	return Socket(s);
}

void Socket::listen(int backlog) {
	if (::listen(fd, backlog) < 0) {
		throw std::system_error(errno, std::system_category(), "listen");
	}
}

ssize_t Socket::recv(void *buf, size_t n, int flags) {
	ssize_t r;
	if ((r = ::recv(fd, buf, n, flags)) < 0) {
		if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
			return 0;
		}
		throw std::system_error(errno, std::system_category(), "recv");
	}
	return r == 0 ? n == 0 ? 0 : -1 : r;
}

size_t Socket::send(const void *buf, size_t n, int flags) {
	ssize_t s;
	if ((s = ::send(fd, buf, n, flags)) < 0) {
		if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
			return 0;
		}
		throw std::system_error(errno, std::system_category(), "send");
	}
	return s;
}

size_t Socket::avail() {
	int n;
	this->ioctl(FIONREAD, &n);
	return n;
}

size_t Socket::write(const void *buf, size_t n) {
	return this->send(buf, n, MSG_MORE | MSG_NOSIGNAL);
}

bool Socket::flush() {
	int optval = 0;
	this->setsockopt(IPPROTO_TCP, TCP_CORK, &optval, static_cast<socklen_t>(sizeof optval));
	return true;
}


template <typename T, typename A>
void SocketBase<T, A>::getsockname(A &addr) const {
	socklen_t addrlen = static_cast<socklen_t>(sizeof addr);
	this->Socket::getsockname(reinterpret_cast<sockaddr *>(&addr), &addrlen);
	assert(addrlen == sizeof addr);
}

template <typename T, typename A>
void SocketBase<T, A>::bind(const A &addr) {
	this->Socket::bind(reinterpret_cast<const sockaddr *>(&addr), static_cast<socklen_t>(sizeof addr));
}

template <typename T, typename A>
void SocketBase<T, A>::getpeername(A &addr) const {
	socklen_t addrlen = static_cast<socklen_t>(sizeof addr);
	this->Socket::getpeername(reinterpret_cast<sockaddr *>(&addr), &addrlen);
	assert(addrlen == sizeof addr);
}

template <typename T, typename A>
bool SocketBase<T, A>::connect(const A &addr) {
	return this->Socket::connect(reinterpret_cast<const sockaddr *>(&addr), static_cast<socklen_t>(sizeof addr));
}

template <typename T, typename A>
T SocketBase<T, A>::accept(A *addr, int flags) {
	socklen_t addrlen = static_cast<socklen_t>(sizeof *addr);
	int fd;
	if ((fd = ::accept4(this->fd, reinterpret_cast<sockaddr *>(addr), addr ? &addrlen : nullptr, flags)) < 0) {
		throw std::system_error(errno, std::system_category(), "accept4");
	}
	return T(fd, nullptr);
}

template class SocketBase<Socket4, sockaddr_in>;
template class SocketBase<Socket6, sockaddr_in6>;
