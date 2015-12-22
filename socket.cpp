#include "socket.h"

#include <cassert>
#include <ostream>
#include <system_error>

#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <sys/ioctl.h>


namespace posix {

int accept(int socket, struct sockaddr * _restrict address, socklen_t * _restrict address_len) {
	int ret;
	if ((ret = ::accept(socket, address, address_len)) < 0) {
		throw std::system_error(errno, std::system_category(), "accept");
	}
	return ret;
}

int accept(int socket, struct sockaddr * _restrict address, socklen_t * _restrict address_len, int flags) {
	int ret;
	if ((ret = ::accept4(socket, address, address_len, flags)) < 0) {
		throw std::system_error(errno, std::system_category(), "accept4");
	}
	return ret;
}

void bind(int socket, const struct sockaddr *address, socklen_t address_len) {
	if (::bind(socket, address, address_len) < 0) {
		throw std::system_error(errno, std::system_category(), "bind");
	}
}

bool connect(int socket, const struct sockaddr *address, socklen_t address_len) {
	if (::connect(socket, address, address_len) < 0) {
		if (errno == EINPROGRESS) {
			return false;
		}
		throw std::system_error(errno, std::system_category(), "connect");
	}
	return true;
}

void getpeername(int socket, struct sockaddr * _restrict address, socklen_t * _restrict address_len) {
	if (::getpeername(socket, address, address_len) < 0) {
		throw std::system_error(errno, std::system_category(), "getpeername");
	}
}

void getsockname(int socket, struct sockaddr * _restrict address, socklen_t * _restrict address_len) {
	if (::getsockname(socket, address, address_len) < 0) {
		throw std::system_error(errno, std::system_category(), "getsockname");
	}
}

void getsockopt(int socket, int level, int option_name, void * _restrict option_value, socklen_t * _restrict option_len) {
	if (::getsockopt(socket, level, option_name, option_value, option_len) < 0) {
		throw std::system_error(errno, std::system_category(), "getsockopt");
	}
}

void listen(int socket, int backlog) {
	if (::listen(socket, backlog) < 0) {
		throw std::system_error(errno, std::system_category(), "listen");
	}
}

ssize_t recv(int socket, void *buffer, size_t length, int flags) {
	ssize_t ret;
	if ((ret = ::recv(socket, buffer, length, flags)) < 0) {
		if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
			return 0;
		}
		throw std::system_error(errno, std::system_category(), "recv");
	}
	return ret == 0 && length > 0 ? -1 : ret;
}

ssize_t recvfrom(int socket, void * _restrict buffer, size_t length, int flags, struct sockaddr * _restrict address, socklen_t * _restrict address_len) {
	ssize_t ret;
	if ((ret = ::recvfrom(socket, buffer, length, flags, address, address_len)) < 0) {
		if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
			return 0;
		}
		throw std::system_error(errno, std::system_category(), "recvfrom");
	}
	return ret == 0 && length > 0 ? -1 : ret;
}

ssize_t recvmsg(int socket, struct msghdr *message, int flags) {
	ssize_t ret;
	if ((ret = ::recvmsg(socket, message, flags)) < 0) {
		if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
			return 0;
		}
		throw std::system_error(errno, std::system_category(), "recvmsg");
	}
	return ret == 0 ? -1 : ret;
}

size_t send(int socket, const void *buffer, size_t length, int flags) {
	ssize_t ret;
	if ((ret = ::send(socket, buffer, length, flags)) < 0) {
		if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
			return 0;
		}
		throw std::system_error(errno, std::system_category(), "send");
	}
	return static_cast<size_t>(ret);
}

size_t sendmsg(int socket, const struct msghdr *message, int flags) {
	ssize_t ret;
	if ((ret = ::sendmsg(socket, message, flags)) < 0) {
		if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
			return 0;
		}
		throw std::system_error(errno, std::system_category(), "sendmsg");
	}
	return static_cast<size_t>(ret);
}

size_t sendto(int socket, const void *message, size_t length, int flags, const struct sockaddr *dest_addr, socklen_t dest_len) {
	ssize_t ret;
	if ((ret = ::sendto(socket, message, length, flags, dest_addr, dest_len)) < 0) {
		if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
			return 0;
		}
		throw std::system_error(errno, std::system_category(), "sendto");
	}
	return static_cast<size_t>(ret);
}

void setsockopt(int socket, int level, int option_name, const void *option_value, socklen_t option_len) {
	if (::setsockopt(socket, level, option_name, option_value, option_len) < 0) {
		throw std::system_error(errno, std::system_category(), "setsockopt");
	}
}

void shutdown(int socket, int how) {
	if (::shutdown(socket, how) < 0) {
		throw std::system_error(errno, std::system_category(), "shutdown");
	}
}

bool sockatmark(int s) {
	int ret;
	if ((ret = ::sockatmark(s)) < 0) {
		throw std::system_error(errno, std::system_category(), "sockatmark");
	}
	return ret != 0;
}

int socket(int domain, int type, int protocol) {
	int ret;
	if ((ret = ::socket(domain, type, protocol)) < 0) {
		throw std::system_error(errno, std::system_category(), "socket");
	}
	return ret;
}

void socketpair(int domain, int type, int protocol, int socket_vector[2]) {
	if (::socketpair(domain, type, protocol, socket_vector) < 0) {
		throw std::system_error(errno, std::system_category(), "socketpair");
	}
}

} // namespace posix


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
	return T(posix::accept(fd, reinterpret_cast<sockaddr *>(addr), addr ? &addrlen : nullptr, flags), nullptr);
}

template class SocketBase<Socket4, sockaddr_in>;
template class SocketBase<Socket6, sockaddr_in6>;


std::ostream & operator << (std::ostream &os, const in_addr &addr) {
	char buf[INET_ADDRSTRLEN];
	if (!::inet_ntop(AF_INET, &addr, buf, sizeof buf)) {
		throw std::system_error(errno, std::system_category(), "inet_ntop");
	}
	return os << buf;
}

std::ostream & operator << (std::ostream &os, const in6_addr &addr) {
	char buf[INET6_ADDRSTRLEN];
	if (!::inet_ntop(AF_INET6, &addr, buf, sizeof buf)) {
		throw std::system_error(errno, std::system_category(), "inet_ntop");
	}
	return os << buf;
}
