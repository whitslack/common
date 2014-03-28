#include "fd.h"

#include <unistd.h>
#include <sys/stat.h>


void FileDescriptor::MemoryMapping::msync(size_t offset, size_t length, int flags) {
	if (::msync(static_cast<uint8_t *>(addr) + offset, length, flags) < 0) {
		throw std::system_error(errno, std::system_category(), "msync");
	}
}

void FileDescriptor::MemoryMapping::madvise(size_t offset, size_t length, int advice) {
	int error;
	if ((error = ::posix_madvise(static_cast<uint8_t *>(addr) + offset, length, advice)) < 0) {
		throw std::system_error(error, std::system_category(), "posix_madvise");
	}
}

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

size_t FileDescriptor::write(const void *buf, size_t n) {
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
