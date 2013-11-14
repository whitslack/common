#include "nbio.h"

#include <cassert>
#include <cstddef>
#include <cstring>
#include <iostream>

#include <netdb.h>
#include <unistd.h>
#include <sys/stat.h>


void FileDescriptor::MemoryMapping::unmap() {
	if (::munmap(addr, length) < 0) {
		throw std::system_error(errno, std::system_category(), "munmap");
	}
}


void FileDescriptor::open(const char pathname[], int flags, mode_t mode) {
	if (fd >= 0) {
		this->close();
	}
	if ((fd = ::open(pathname, flags, mode)) < 0) {
		throw std::system_error(errno, std::system_category(), pathname);
	}
}

void FileDescriptor::close() {
	if (::close(fd) < 0) {
		throw std::system_error(errno, std::system_category(), "close");
	}
	fd = -1;
}

ssize_t FileDescriptor::read(void *buf, size_t n) {
	ssize_t r;
	if ((r = ::read(fd, buf, n)) < 0) {
		if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
			return 0;
		}
		throw std::system_error(errno, std::system_category(), "read");
	}
	return r == 0 ? n == 0 ? 0 : -1 : r;
}

size_t FileDescriptor::write(const void *buf, size_t n, bool) {
	ssize_t w;
	if ((w = ::write(fd, buf, n)) < 0) {
		if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
			return 0;
		}
		throw std::system_error(errno, std::system_category(), "write");
	}
	return w;
}

off_t FileDescriptor::lseek(off_t offset, int whence) {
	off_t p;
	if ((p = ::lseek(fd, offset, whence)) < 0) {
		throw std::system_error(errno, std::system_category(), "lseek");
	}
	return p;
}

void FileDescriptor::fstat(struct stat *st) {
	if (::fstat(fd, st) < 0) {
		throw std::system_error(errno, std::system_category(), "fstat");
	}
}

void FileDescriptor::fchmod(mode_t mode) {
	if (::fchmod(fd, mode) < 0) {
		throw std::system_error(errno, std::system_category(), "fchmod");
	}
}

void FileDescriptor::fallocate(off_t offset, off_t length) {
	int error;
	if ((error = ::posix_fallocate(fd, offset, length)) < 0) {
		throw std::system_error(error, std::system_category(), "posix_fallocate");
	}
}

void FileDescriptor::ftruncate(off_t length) {
	if (::ftruncate(fd, length) < 0) {
		throw std::system_error(errno, std::system_category(), "ftruncate");
	}
}

void FileDescriptor::fsync() {
	if (::fsync(fd) < 0) {
		throw std::system_error(errno, std::system_category(), "fsync");
	}
}

void FileDescriptor::fdatasync() {
	if (::fdatasync(fd) < 0) {
		throw std::system_error(errno, std::system_category(), "fdatasync");
	}
}

void FileDescriptor::fadvise(off_t offset, off_t length, int advice) {
	int error;
	if ((error = ::posix_fadvise(fd, offset, length, advice)) < 0) {
		throw std::system_error(error, std::system_category(), "posix_fadvise");
	}
}

FileDescriptor::MemoryMapping FileDescriptor::mmap(off_t offset, size_t length, int prot, int flags) {
	void *addr;
	if ((addr = ::mmap(nullptr, length, prot, flags, fd, offset)) == MAP_FAILED) {
		throw std::system_error(errno, std::system_category(), "mmap");
	}
	return { addr, length };
}


EPoll::EPoll() : FileDescriptor(::epoll_create1(EPOLL_CLOEXEC)) {
	if (fd < 0) {
		throw std::system_error(errno, std::system_category(), "epoll_create1");
	}
}

void EPoll::pump() {
	epoll_event event;
	int n;
	if ((n = ::epoll_wait(fd, &event, 1, -1)) < 0 && errno != EINTR) {
		throw std::system_error(errno, std::system_category(), "epoll_wait");
	}
	if (n > 0) {
		static_cast<EPollable *>(event.data.ptr)->ready(*this, event.events);
	}
}

void EPoll::watch(EPollable *epollable, uint32_t events, int op) {
	epoll_event event;
	event.events = events | EPOLLONESHOT;
	event.data.ptr = epollable;
	if (::epoll_ctl(fd, op, *epollable, &event) < 0) {
		throw std::system_error(errno, std::system_category(), "epoll_ctl");
	}
}


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
	return Socket(s, nullptr);
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

size_t Socket::write(const void *buf, size_t n, bool more) {
	return this->send(buf, n, more ? MSG_MORE | MSG_NOSIGNAL : MSG_NOSIGNAL);
}

bool Socket::finish() {
	this->write(nullptr, 0, false);
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


const char * GAICategory::name() const noexcept {
	return "GAI";
}

std::string GAICategory::message(int condition) const noexcept {
	return ::gai_strerror(condition);
}


GAIResults getaddrinfo(const char host[], const char service[], int family, int type, int protocol, int flags) {
	addrinfo hints;
	std::memset(&hints, 0, sizeof hints);
	hints.ai_family = family;
	hints.ai_socktype = type;
	hints.ai_protocol = protocol;
	hints.ai_flags = flags;
	addrinfo *res;
	int error;
	if ((error = ::getaddrinfo(host, service, &hints, &res)) != 0) {
		if (error == EAI_SYSTEM) {
			throw std::system_error(errno, std::system_category(), "getaddrinfo");
		}
		else {
			throw std::system_error(error, GAICategory(), "getaddrinfo");
		}
	}
	return GAIResults(res);
}


std::ostream & operator << (std::ostream &os, const sockaddr_in &addr) {
	char host[NI_MAXHOST], serv[NI_MAXSERV];
	int error;
	if ((error = ::getnameinfo(reinterpret_cast<const sockaddr *>(&addr), static_cast<socklen_t>(sizeof addr), host, static_cast<socklen_t>(sizeof host), serv, static_cast<socklen_t>(sizeof serv), NI_NUMERICHOST | NI_NUMERICSERV)) != 0) {
		if (error == EAI_SYSTEM) {
			throw std::system_error(errno, std::system_category(), "getnameinfo");
		}
		else {
			throw std::system_error(error, GAICategory(), "getnameinfo");
		}
	}
	return os << host << ':' << serv;
}

std::ostream & operator << (std::ostream &os, const sockaddr_in6 &addr) {
	char host[NI_MAXHOST], serv[NI_MAXSERV];
	int error;
	if ((error = ::getnameinfo(reinterpret_cast<const sockaddr *>(&addr), static_cast<socklen_t>(sizeof addr), host, static_cast<socklen_t>(sizeof host), serv, static_cast<socklen_t>(sizeof serv), NI_NUMERICHOST | NI_NUMERICSERV)) != 0) {
		if (error == EAI_SYSTEM) {
			throw std::system_error(errno, std::system_category(), "getnameinfo");
		}
		else {
			throw std::system_error(error, GAICategory(), "getnameinfo");
		}
	}
	return os << '[' << host << "]:" << serv;
}
