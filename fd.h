#pragma once

#include <system_error>

#include <fcntl.h>
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
		void msync(size_t offset, size_t length, int flags = MS_ASYNC);
		void madvise(size_t offset, size_t length, int advice);
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
	size_t write(const void *buf, size_t n) override;
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
