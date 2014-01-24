#pragma once

#include <cstdint>
#include <streambuf>
#include <system_error>
#include <utility>

#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include "io.h"


class FileDescriptor : public Source, public Sink {
	friend class EPoll;

public:
	class MemoryMapping {
		friend FileDescriptor;
	private:
		void *addr;
		size_t length;
	public:
		MemoryMapping(MemoryMapping &&move) : addr(move.addr), length(move.length) { move.addr = MAP_FAILED; }
		MemoryMapping & operator = (MemoryMapping &&move) { std::swap(addr, move.addr); std::swap(length, move.length); return *this; }
		~MemoryMapping() { if (addr != MAP_FAILED) this->unmap(); }
	private:
		MemoryMapping(void *addr, size_t length) : addr(addr), length(length) { }
		MemoryMapping(const MemoryMapping &) = delete;
		MemoryMapping & operator = (const MemoryMapping &) = delete;
	public:
		operator void * () const { return addr; }
		void * data() const { return addr; }
		size_t size() const { return length; }
	private:
		void unmap();
	};

protected:
	int fd;

public:
	FileDescriptor() : fd(-1) { }
	explicit FileDescriptor(int fd) : fd(fd) { }
	explicit FileDescriptor(const char pathname[], int flags = O_RDONLY | O_CLOEXEC, mode_t mode = 0666) : fd(-1) { this->open(pathname, flags, mode); }
	FileDescriptor(FileDescriptor &&move) : fd(move.fd) { move.fd = -1; }
	FileDescriptor & operator = (FileDescriptor &&move) { std::swap(fd, move.fd); return *this; }
	virtual ~FileDescriptor() { if (fd >= 0) this->close(); }

public:
	operator int () const { return fd; }
	void open(const char pathname[], int flags = O_RDONLY | O_CLOEXEC, mode_t mode = 0666);
	void close();
	ssize_t read(void *buf, size_t n) override;
	size_t write(const void *buf, size_t n, bool more = false) override;
	off_t lseek(off_t offset, int whence = SEEK_SET);
	void fstat(struct stat *st);
	void fchmod(mode_t mode);
	void fallocate(off_t offset, off_t length);
	void ftruncate(off_t length);
	void fsync();
	void fdatasync();
	void fadvise(off_t offset, off_t length, int advice);
	MemoryMapping mmap(off_t offset, size_t length, int prot = PROT_READ, int flags = MAP_SHARED);

	template <typename... Args>
	int fcntl(int cmd, Args&&... args) {
		int r;
		if ((r = ::fcntl(fd, cmd, std::forward<Args>(args)...)) < 0) {
			throw std::system_error(errno, std::system_category(), "fcntl");
		}
		return r;
	}

	template <typename... Args>
	int ioctl(int request, Args&&... args) {
		int r;
		if ((r = ::ioctl(fd, request, std::forward<Args>(args)...)) < 0) {
			throw std::system_error(errno, std::system_category(), "ioctl");
		}
		return r;
	}

private:
	FileDescriptor(const FileDescriptor &) = delete;
	FileDescriptor & operator = (const FileDescriptor &) = delete;

};


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

	void pump();
	void watch(EPollable *epollable, uint32_t events, int op = EPOLL_CTL_MOD);

};


class Socket : public FileDescriptor {

public:
	Socket() { }
	Socket(int domain, int type, int protocol = 0) { this->open(domain, type, protocol); }

protected:
	Socket(int fd, std::nullptr_t) : FileDescriptor(fd) { }

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
	size_t write(const void *buf, size_t n, bool more = false) override;
	bool finish() override;

};


template <typename T, typename A>
class SocketBase : public Socket {

protected:
	SocketBase() { }
	SocketBase(int fd) : Socket(fd, nullptr) { }

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
	Socket4(int type, int protocol = 0, int flags = SOCK_NONBLOCK | SOCK_CLOEXEC) { this->open(type, protocol, flags); }

protected:
	Socket4(int fd, std::nullptr_t) : SocketBase<Socket4, sockaddr_in>(fd) { }

public:
	void open(int type = SOCK_STREAM, int protocol = 0, int flags = SOCK_NONBLOCK | SOCK_CLOEXEC) { this->SocketBase::open(AF_INET, type | flags, protocol); }

};


class Socket6 : public SocketBase<Socket6, sockaddr_in6> {
	friend SocketBase<Socket6, sockaddr_in6>;

public:
	Socket6() { }
	Socket6(int type, int protocol = 0, int flags = SOCK_NONBLOCK | SOCK_CLOEXEC) { this->open(type, protocol, flags); }

protected:
	Socket6(int fd, std::nullptr_t) : SocketBase<Socket6, sockaddr_in6>(fd) { }

public:
	void open(int type = SOCK_STREAM, int protocol = 0, int flags = SOCK_NONBLOCK | SOCK_CLOEXEC) { this->SocketBase::open(AF_INET6, type | flags, protocol); }

};


class GAICategory : public std::error_category {

public:
	const char * name() const noexcept override _const;
	std::string message(int condition) const noexcept override _const;

};


class GAIResults {
	friend GAIResults getaddrinfo(const char [], const char [], int, int, int, int);

public:
	class Iterator : public std::forward_iterator_tag {
		friend class GAIResults;
	private:
		addrinfo *ptr;
	public:
		Iterator() : ptr() { }
		operator addrinfo * () const { return ptr; }
		addrinfo * operator -> () const { return ptr; }
		Iterator & operator ++ () {
			ptr = ptr->ai_next;
			return *this;
		}
		Iterator operator ++ (int) {
			Iterator copy(*this);
			ptr = ptr->ai_next;
			return copy;
		}
	private:
		explicit Iterator(addrinfo *ptr) : ptr(ptr) { }
	};

private:
	addrinfo *res;

public:
	GAIResults(GAIResults &&move) : res(move.res) { move.res = nullptr; }
	~GAIResults() { ::freeaddrinfo(res); }

	Iterator begin() const { return Iterator(res); }
	Iterator end() const { return Iterator(); }

private:
	explicit GAIResults(addrinfo *res) : res(res) { }
	GAIResults(const GAIResults &) = delete;
	GAIResults & operator = (const GAIResults &) = delete;

};

GAIResults getaddrinfo(const char host[], const char service[] = nullptr, int family = AF_UNSPEC, int type = SOCK_STREAM, int protocol = 0, int flags = AI_V4MAPPED | AI_ADDRCONFIG);


std::ostream & operator << (std::ostream &os, const sockaddr_in &addr);
std::ostream & operator << (std::ostream &os, const sockaddr_in6 &addr);
