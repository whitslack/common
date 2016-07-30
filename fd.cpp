#include "fd.h"

#include <cstdio>
#include <system_error>

#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/time.h>


namespace posix {

void access(const char *path, int amode) {
	if (::access(path, amode) < 0) {
		throw std::system_error(errno, std::system_category(), "access");
	}
}

void chdir(const char *path) {
	if (::chdir(path) < 0) {
		throw std::system_error(errno, std::system_category(), "chdir");
	}
}

void chmod(const char *path, mode_t mode) {
	if (::chmod(path, mode) < 0) {
		throw std::system_error(errno, std::system_category(), "chmod");
	}
}

void chown(const char *path, uid_t owner, gid_t group) {
	if (::chown(path, owner, group) < 0) {
		throw std::system_error(errno, std::system_category(), "chown");
	}
}

void close(int fildes) {
	if (::close(fildes) < 0) {
		throw std::system_error(errno, std::system_category(), "close");
	}
}

int creat(const char *path, mode_t mode) {
	int ret;
	if ((ret = ::creat(path, mode)) < 0) {
		throw std::system_error(errno, std::system_category(), "creat");
	}
	return ret;
}

int dup(int fildes) {
	int ret;
	if ((ret = ::dup(fildes)) < 0) {
		throw std::system_error(errno, std::system_category(), "dup");
	}
	return ret;
}

int dup2(int fildes, int fildes2) {
	int ret;
	if ((ret = ::dup2(fildes, fildes2)) < 0) {
		throw std::system_error(errno, std::system_category(), "dup2");
	}
	return ret;
}

void faccessat(int fd, const char *path, int amode, int flag) {
	if (::faccessat(fd, path, amode, flag) < 0) {
		throw std::system_error(errno, std::system_category(), "faccessat");
	}
}

void fadvise(int fd, off_t offset, off_t len, int advice) {
	int error;
	if ((error = ::posix_fadvise(fd, offset, len, advice)) != 0) {
		throw std::system_error(error, std::system_category(), "posix_fadvise");
	}
}

void fallocate(int fd, off_t offset, off_t len) {
	int error;
	if ((error = ::posix_fallocate(fd, offset, len)) != 0) {
		throw std::system_error(error, std::system_category(), "posix_fallocate");
	}
}

void fchdir(int fildes) {
	if (::fchdir(fildes) < 0) {
		throw std::system_error(errno, std::system_category(), "fchdir");
	}
}

void fchmod(int fildes, mode_t mode) {
	if (::fchmod(fildes, mode) < 0) {
		throw std::system_error(errno, std::system_category(), "fchmod");
	}
}

void fchmodat(int fd, const char *path, mode_t mode, int flag) {
	if (::fchmodat(fd, path, mode, flag) < 0) {
		throw std::system_error(errno, std::system_category(), "fchmodat");
	}
}

void fchown(int fildes, uid_t owner, gid_t group) {
	if (::fchown(fildes, owner, group) < 0) {
		throw std::system_error(errno, std::system_category(), "fchown");
	}
}

void fchownat(int fd, const char *path, uid_t owner, gid_t group, int flag) {
	if (::fchownat(fd, path, owner, group, flag) < 0) {
		throw std::system_error(errno, std::system_category(), "fchownat");
	}
}

int fcntl(int fildes, int cmd) {
	int ret;
	if ((ret = ::fcntl(fildes, cmd)) == -1) {
		throw std::system_error(errno, std::system_category(), "fcntl");
	}
	return ret;
}

int fcntl(int fildes, int cmd, int arg) {
	int ret;
	if ((ret = ::fcntl(fildes, cmd, arg)) == -1) {
		throw std::system_error(errno, std::system_category(), "fcntl");
	}
	return ret;
}

int fcntl(int fildes, int cmd, void *arg) {
	int ret;
	if ((ret = ::fcntl(fildes, cmd, arg)) == -1) {
		throw std::system_error(errno, std::system_category(), "fcntl");
	}
	return ret;
}

void fdatasync(int fildes) {
	if (::fdatasync(fildes) < 0) {
		throw std::system_error(errno, std::system_category(), "fdatasync");
	}
}

void fstat(int fildes, struct stat *buf) {
	if (::fstat(fildes, buf) < 0) {
		throw std::system_error(errno, std::system_category(), "fstat");
	}
}

void fstatat(int fd, const char * _restrict path, struct stat * _restrict buf, int flag) {
	if (::fstatat(fd, path, buf, flag) < 0) {
		throw std::system_error(errno, std::system_category(), "fstatat");
	}
}

void fsync(int fildes) {
	if (::fsync(fildes) < 0) {
		throw std::system_error(errno, std::system_category(), "fsync");
	}
}

void ftruncate(int fildes, off_t length) {
	if (::ftruncate(fildes, length) < 0) {
		throw std::system_error(errno, std::system_category(), "ftruncate");
	}
}

void futimens(int fd, const struct timespec times[2]) {
	if (::futimens(fd, times) < 0) {
		throw std::system_error(errno, std::system_category(), "futimens");
	}
}

int ioctl(int fildes, int request) {
	int ret;
	if ((ret = ::ioctl(fildes, request)) == -1) {
		throw std::system_error(errno, std::system_category(), "ioctl");
	}
	return ret;
}

int ioctl(int fildes, int request, int arg) {
	int ret;
	if ((ret = ::ioctl(fildes, request, arg)) == -1) {
		throw std::system_error(errno, std::system_category(), "ioctl");
	}
	return ret;
}

int ioctl(int fildes, int request, void *arg) {
	int ret;
	if ((ret = ::ioctl(fildes, request, arg)) == -1) {
		throw std::system_error(errno, std::system_category(), "ioctl");
	}
	return ret;
}

bool isatty(int fildes) {
	int ret;
	errno = 0;
	if ((ret = ::isatty(fildes)) == 0 && errno != ENOTTY) {
		throw std::system_error(errno, std::system_category(), "isatty");
	}
	return ret != 0;
}

void lchown(const char *path, uid_t owner, gid_t group) {
	if (::lchown(path, owner, group) < 0) {
		throw std::system_error(errno, std::system_category(), "lchown");
	}
}

void link(const char *path1, const char *path2) {
	if (::link(path1, path2) < 0) {
		throw std::system_error(errno, std::system_category(), "link");
	}
}

void linkat(int fd1, const char *path1, int fd2, const char *path2, int flag) {
	if (::linkat(fd1, path1, fd2, path2, flag) < 0) {
		throw std::system_error(errno, std::system_category(), "linkat");
	}
}

void lockf(int fildes, int function, off_t size) {
	if (::lockf(fildes, function, size) < 0) {
		throw std::system_error(errno, std::system_category(), "lockf");
	}
}

off_t lseek(int fildes, off_t offset, int whence) {
	off_t ret;
	if ((ret = ::lseek(fildes, offset, whence)) < 0) {
		throw std::system_error(errno, std::system_category(), "lseek");
	}
	return ret;
}

void lstat(const char * _restrict path, struct stat * _restrict buf) {
	if (::lstat(path, buf) < 0) {
		throw std::system_error(errno, std::system_category(), "lstat");
	}
}

void madvise(void *addr, size_t len, int advice) {
	int error;
	if ((error = ::posix_madvise(addr, len, advice)) != 0) {
		throw std::system_error(error, std::system_category(), "posix_madvise");
	}
}

void mkdir(const char *path, mode_t mode) {
	if (::mkdir(path, mode) < 0) {
		throw std::system_error(errno, std::system_category(), "mkdir");
	}
}

void mkdirat(int fd, const char *path, mode_t mode) {
	if (::mkdirat(fd, path, mode) < 0) {
		throw std::system_error(errno, std::system_category(), "mkdirat");
	}
}

void mkfifo(const char *path, mode_t mode) {
	if (::mkfifo(path, mode) < 0) {
		throw std::system_error(errno, std::system_category(), "mkfifo");
	}
}

void mkfifoat(int fd, const char *path, mode_t mode) {
	if (::mkfifoat(fd, path, mode) < 0) {
		throw std::system_error(errno, std::system_category(), "mkfifoat");
	}
}

void mknod(const char *path, mode_t mode, dev_t dev) {
	if (::mknod(path, mode, dev) < 0) {
		throw std::system_error(errno, std::system_category(), "mknod");
	}
}

void mknodat(int fd, const char *path, mode_t mode, dev_t dev) {
	if (::mknodat(fd, path, mode, dev) < 0) {
		throw std::system_error(errno, std::system_category(), "mknodat");
	}
}

void * mmap(void *addr, size_t len, int prot, int flags, int fildes, off_t off) {
	void *ret;
	if ((ret = ::mmap(addr, len, prot, flags, fildes, off)) == MAP_FAILED) {
		throw std::system_error(errno, std::system_category(), "mmap");
	}
	return ret;
}

void mprotect(void *addr, size_t len, int prot) {
	if (::mprotect(addr, len, prot) < 0) {
		throw std::system_error(errno, std::system_category(), "mprotect");
	}
}

void msync(void *addr, size_t len, int flags) {
	if (::msync(addr, len, flags) < 0) {
		throw std::system_error(errno, std::system_category(), "msync");
	}
}

void munmap(void *addr, size_t len) {
	if (::munmap(addr, len) < 0) {
		throw std::system_error(errno, std::system_category(), "munmap");
	}
}

int open(const char *path, int oflag, mode_t mode) {
	int ret;
	if ((ret = ::open(path, oflag, mode)) < 0) {
		throw std::system_error(errno, std::system_category(), "open");
	}
	return ret;
}

int openat(int fd, const char *path, int oflag, mode_t mode) {
	int ret;
	if ((ret = ::openat(fd, path, oflag, mode)) < 0) {
		throw std::system_error(errno, std::system_category(), "openat");
	}
	return ret;
}

void pipe(int fildes[2]) {
	if (::pipe(fildes) < 0) {
		throw std::system_error(errno, std::system_category(), "pipe");
	}
}

unsigned poll(struct pollfd fds[], nfds_t nfds, int timeout) {
	int ret;
	if ((ret = ::poll(fds, nfds, timeout)) < 0) {
		if (errno == EINTR) {
			return 0;
		}
		throw std::system_error(errno, std::system_category(), "poll");
	}
	return static_cast<unsigned>(ret);
}

ssize_t pread(int fildes, void *buf, size_t nbyte, off_t offset) {
	ssize_t ret;
	if ((ret = ::pread(fildes, buf, nbyte, offset)) < 0) {
		if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
			return 0;
		}
		throw std::system_error(errno, std::system_category(), "pread");
	}
	return ret == 0 && nbyte > 0 ? -1 : ret;
}

ssize_t preadv(int fildes, const struct iovec iov[], int iovcnt, off_t offset) {
	ssize_t ret;
	if ((ret = ::preadv(fildes, iov, iovcnt, offset)) < 0) {
		if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
			return 0;
		}
		throw std::system_error(errno, std::system_category(), "preadv");
	}
	if (ret == 0) {
		for (int i = 0; i < iovcnt; ++i) {
			if (iov[i].iov_len > 0) {
				return -1;
			}
		}
	}
	return ret;
}

size_t pwrite(int fildes, const void *buf, size_t nbyte, off_t offset) {
	ssize_t ret;
	if ((ret = ::pwrite(fildes, buf, nbyte, offset)) < 0) {
		if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
			return 0;
		}
		throw std::system_error(errno, std::system_category(), "pwrite");
	}
	return static_cast<size_t>(ret);
}

size_t pwritev(int fildes, const struct iovec iov[], int iovcnt, off_t offset) {
	ssize_t ret;
	if ((ret = ::pwritev(fildes, iov, iovcnt, offset)) < 0) {
		if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
			return 0;
		}
		throw std::system_error(errno, std::system_category(), "pwritev");
	}
	return static_cast<size_t>(ret);
}

ssize_t read(int fildes, void *buf, size_t nbyte) {
	ssize_t ret;
	if ((ret = ::read(fildes, buf, nbyte)) < 0) {
		if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
			return 0;
		}
		throw std::system_error(errno, std::system_category(), "read");
	}
	return ret == 0 && nbyte > 0 ? -1 : ret;
}

ssize_t readv(int fildes, const struct iovec iov[], int iovcnt) {
	ssize_t ret;
	if ((ret = ::readv(fildes, iov, iovcnt)) < 0) {
		if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
			return 0;
		}
		throw std::system_error(errno, std::system_category(), "readv");
	}
	if (ret == 0) {
		for (int i = 0; i < iovcnt; ++i) {
			if (iov[i].iov_len > 0) {
				return -1;
			}
		}
	}
	return ret;
}

size_t readlink(const char * _restrict path, char * _restrict buf, size_t bufsize) {
	ssize_t ret;
	if ((ret = ::readlink(path, buf, bufsize)) < 0) {
		throw std::system_error(errno, std::system_category(), "readlink");
	}
	return static_cast<size_t>(ret);
}

size_t readlinkat(int fd, const char * _restrict path, char * _restrict buf, size_t bufsize) {
	ssize_t ret;
	if ((ret = ::readlinkat(fd, path, buf, bufsize)) < 0) {
		throw std::system_error(errno, std::system_category(), "readlinkat");
	}
	return static_cast<size_t>(ret);
}

void rename(const char *oldpath, const char *newpath) {
	if (::rename(oldpath, newpath) < 0) {
		throw std::system_error(errno, std::system_category(), "rename");
	}
}

void renameat(int oldfd, const char *oldpath, int newfd, const char *newpath) {
	if (::renameat(oldfd, oldpath, newfd, newpath) < 0) {
		throw std::system_error(errno, std::system_category(), "renameat");
	}
}

void rmdir(const char *path) {
	if (::rmdir(path) < 0) {
		throw std::system_error(errno, std::system_category(), "rmdir");
	}
}

unsigned select(int nfds, fd_set * _restrict readfds, fd_set * _restrict writefds, fd_set * _restrict errorfds, struct timeval * _restrict timeout) {
	int ret;
	if ((ret = ::select(nfds, readfds, writefds, errorfds, timeout)) < 0) {
		if (errno == EINTR) {
			return 0;
		}
		throw std::system_error(errno, std::system_category(), "select");
	}
	return static_cast<unsigned>(ret);
}

void stat(const char * _restrict path, struct stat * _restrict buf) {
	if (::stat(path, buf) < 0) {
		throw std::system_error(errno, std::system_category(), "stat");
	}
}

void symlink(const char *path1, const char *path2) {
	if (::symlink(path1, path2) < 0) {
		throw std::system_error(errno, std::system_category(), "symlink");
	}
}

void symlinkat(const char *path1, int fd, const char *path2) {
	if (::symlinkat(path1, fd, path2) < 0) {
		throw std::system_error(errno, std::system_category(), "symlinkat");
	}
}

void truncate(const char *path, off_t length) {
	if (::truncate(path, length) < 0) {
		throw std::system_error(errno, std::system_category(), "truncate");
	}
}

void unlink(const char *path) {
	if (::unlink(path) < 0) {
		throw std::system_error(errno, std::system_category(), "unlink");
	}
}

void unlinkat(int fd, const char *path, int flag) {
	if (::unlinkat(fd, path, flag) < 0) {
		throw std::system_error(errno, std::system_category(), "unlinkat");
	}
}

void utimensat(int fd, const char *path, const struct timespec times[2], int flag) {
	if (::utimensat(fd, path, times, flag) < 0) {
		throw std::system_error(errno, std::system_category(), "utimensat");
	}
}

void utimes(const char *path, const struct timeval times[2]) {
	if (::utimes(path, times) < 0) {
		throw std::system_error(errno, std::system_category(), "utimes");
	}
}

size_t write(int fildes, const void *buf, size_t nbyte) {
	ssize_t ret;
	if ((ret = ::write(fildes, buf, nbyte)) < 0) {
		if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
			return 0;
		}
		throw std::system_error(errno, std::system_category(), "write");
	}
	return static_cast<size_t>(ret);
}

size_t writev(int fildes, const struct iovec iov[], int iovcnt) {
	ssize_t ret;
	if ((ret = ::writev(fildes, iov, iovcnt)) < 0) {
		if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
			return 0;
		}
		throw std::system_error(errno, std::system_category(), "writev");
	}
	return static_cast<size_t>(ret);
}

} // namespace posix


void FileDescriptor::pread_fully(void *buf, size_t nbyte, off_t offset) const {
	while (nbyte > 0) {
		ssize_t r = this->pread(buf, nbyte, offset);
		if (r > 0) {
			buf = static_cast<uint8_t *>(buf) + r, nbyte -= r, offset += r;
		}
		else if (r < 0) {
			throw std::ios_base::failure("premature EOF");
		}
		else {
			throw std::logic_error("non-blocking read in blocking context");
		}
	}
}

void FileDescriptor::pwrite_fully(const void *buf, size_t nbyte, off_t offset) {
	while (nbyte > 0) {
		size_t w = this->pwrite(buf, nbyte, offset);
		if (w > 0) {
			buf = static_cast<const uint8_t *>(buf) + w, nbyte -= w, offset += w;
		}
		else {
			throw std::logic_error("non-blocking write in blocking context");
		}
	}
}

void FileDescriptor::readv_fully(struct iovec iov[], int iovcnt) {
	while (iovcnt > 0) {
		while (iov->iov_len == 0) {
			++iov;
			if (--iovcnt == 0) {
				return;
			}
		}
		ssize_t r = this->readv(iov, iovcnt);
		if (r > 0) {
			while (static_cast<size_t>(r) > iov->iov_len) {
				r -= iov->iov_len;
				iov->iov_base = static_cast<char *>(iov->iov_base) + iov->iov_len;
				iov->iov_len = 0;
				++iov, --iovcnt;
			}
			iov->iov_base = static_cast<char *>(iov->iov_base) + r;
			if ((iov->iov_len -= r) == 0) {
				++iov, --iovcnt;
			}
		}
		else if (r < 0) {
			throw std::ios_base::failure("premature EOF");
		}
		else {
			throw std::logic_error("non-blocking read in blocking context");
		}
	}
}

void FileDescriptor::writev_fully(struct iovec iov[], int iovcnt) {
	while (iovcnt > 0) {
		while (iov->iov_len == 0) {
			++iov;
			if (--iovcnt == 0) {
				return;
			}
		}
		size_t w = this->writev(iov, iovcnt);
		if (w > 0) {
			while (w > iov->iov_len) {
				w -= iov->iov_len;
				iov->iov_base = static_cast<char *>(iov->iov_base) + iov->iov_len;
				iov->iov_len = 0;
				++iov, --iovcnt;
			}
			iov->iov_base = static_cast<char *>(iov->iov_base) + w;
			if ((iov->iov_len -= w) == 0) {
				++iov, --iovcnt;
			}
		}
		else {
			throw std::logic_error("non-blocking write in blocking context");
		}
	}
}

void FileDescriptor::preadv_fully(struct iovec iov[], int iovcnt, off_t offset) const {
	while (iovcnt > 0) {
		while (iov->iov_len == 0) {
			++iov;
			if (--iovcnt == 0) {
				return;
			}
		}
		ssize_t r = this->preadv(iov, iovcnt, offset);
		if (r > 0) {
			offset += r;
			while (static_cast<size_t>(r) > iov->iov_len) {
				r -= iov->iov_len;
				iov->iov_base = static_cast<char *>(iov->iov_base) + iov->iov_len;
				iov->iov_len = 0;
				++iov, --iovcnt;
			}
			iov->iov_base = static_cast<char *>(iov->iov_base) + r;
			if ((iov->iov_len -= r) == 0) {
				++iov, --iovcnt;
			}
		}
		else if (r < 0) {
			throw std::ios_base::failure("premature EOF");
		}
		else {
			throw std::logic_error("non-blocking read in blocking context");
		}
	}
}

void FileDescriptor::pwritev_fully(struct iovec iov[], int iovcnt, off_t offset) {
	while (iovcnt > 0) {
		while (iov->iov_len == 0) {
			++iov;
			if (--iovcnt == 0) {
				return;
			}
		}
		size_t w = this->pwritev(iov, iovcnt, offset);
		if (w > 0) {
			offset += w;
			while (w > iov->iov_len) {
				w -= iov->iov_len;
				iov->iov_base = static_cast<char *>(iov->iov_base) + iov->iov_len;
				iov->iov_len = 0;
				++iov, --iovcnt;
			}
			iov->iov_base = static_cast<char *>(iov->iov_base) + w;
			if ((iov->iov_len -= w) == 0) {
				++iov, --iovcnt;
			}
		}
		else {
			throw std::logic_error("non-blocking write in blocking context");
		}
	}
}


#if _POSIX_VERSION < 200809L

#ifdef __APPLE__

#include <stdarg.h>

#include <signal.h>
#include <sys/syscall.h>

#include "auto_cast.h"
#include "memory.h"
#include "narrow.h"

#if _REENTRANT
#	define _sigmask pthread_sigmask
#else
#	define _sigmask sigprocmask
#endif

#define _at(_fd, _call) ({ \
	decltype(_call) ret = -1; \
	sigset_t all_signals, prev_mask; \
	if (sigfillset(&all_signals) == 0 && ::_sigmask(SIG_BLOCK, &all_signals, &prev_mask) == 0) { \
		if (::syscall(SYS___pthread_fchdir, _fd) == 0) { \
			ret = (_call); \
			if (::syscall(SYS___pthread_fchdir, -1) < 0) { \
				ret = -1; \
			} \
		} \
		if (::_sigmask(SIG_SETMASK, &prev_mask, nullptr) < 0) { \
			ret = -1; \
		} \
	} \
	ret; })

static int utimens_to_utimes(struct timeval **ptvs, const struct timespec times[2]) {
	if (!times || (times[0].tv_nsec == UTIME_NOW && times[1].tv_nsec == UTIME_NOW)) {
		*ptvs = nullptr;
		return 0;
	}
	if (times[0].tv_nsec == UTIME_OMIT || times[1].tv_nsec == UTIME_OMIT) {
		errno = ENOTSUP;
		return -1;
	}
	struct timeval *tvs = *ptvs;
	if (times[0].tv_nsec == UTIME_NOW) {
		if (::gettimeofday(&tvs[0], nullptr) < 0) {
			return -1;
		}
		if (times[1].tv_nsec == UTIME_NOW) {
			tvs[1] = tvs[0];
			return 0;
		}
	}
	else {
		tvs[0].tv_sec = times[0].tv_sec;
		tvs[0].tv_usec = auto_cast(times[0].tv_nsec / 1000);
	}
	if (times[1].tv_nsec == UTIME_NOW) {
		if (::gettimeofday(&tvs[1], nullptr) < 0) {
			return -1;
		}
	}
	else {
		tvs[1].tv_sec = times[1].tv_sec;
		tvs[1].tv_usec = auto_cast(times[1].tv_nsec / 1000);
	}
	return 0;
}

int faccessat(int fd, const char *path, int amode, int flag) {
	if (flag != 0) {
		errno = flag == AT_EACCESS ? ENOTSUP : EINVAL;
		return -1;
	}
	return fd == AT_FDCWD || *path == '/' ? ::access(path, amode) : _at(fd, ::access(path, amode));
}

int fchmodat(int fd, const char *path, mode_t mode, int flag) {
	if (flag != 0) {
		if (flag == AT_SYMLINK_NOFOLLOW) {
			int ret = -1;
			if ((fd = ::openat(fd, path, O_RDONLY | O_SYMLINK | O_CLOEXEC)) >= 0) {
				ret = ::fchmod(fd, mode);
				if (::close(fd) < 0) {
					ret = -1;
				}
			}
			return ret;
		}
		errno = EINVAL;
		return -1;
	}
	return fd == AT_FDCWD || *path == '/' ? ::chmod(path, mode) : _at(fd, ::chmod(path, mode));
}

int fchownat(int fd, const char *path, uid_t owner, gid_t group, int flag) {
	if (flag != 0) {
		if (flag == AT_SYMLINK_NOFOLLOW) {
			return fd == AT_FDCWD || *path == '/' ? ::lchown(path, owner, group) : _at(fd, ::lchown(path, owner, group));
		}
		errno = EINVAL;
		return -1;
	}
	return fd == AT_FDCWD || *path == '/' ? ::chown(path, owner, group) : _at(fd, ::chown(path, owner, group));
}

int fstatat(int fd, const char * _restrict path, struct stat * _restrict buf, int flag) {
	if (flag != 0) {
		if (flag == AT_SYMLINK_NOFOLLOW) {
			return fd == AT_FDCWD || *path == '/' ? ::lstat(path, buf) : _at(fd, ::lstat(path, buf));
		}
		errno = EINVAL;
		return -1;
	}
	return fd == AT_FDCWD || *path == '/' ? ::stat(path, buf) : _at(fd, ::stat(path, buf));
}

int futimens(int fd, const struct timespec times[2]) {
	struct timeval tvs[2], *ptvs = tvs;
	return ::utimens_to_utimes(&ptvs, times) < 0 ? -1 : ::futimes(fd, ptvs);
}

int linkat(int fd1, const char *path1, int fd2, const char *path2, int flag) {
	if (flag != AT_SYMLINK_FOLLOW) {
		errno = flag == 0 ? ENOTSUP : EINVAL;
		return -1;
	}
	if (fd1 != fd2 && *path1 != '/' && *path2 != '/') {
		errno = ENOTSUP;
		return -1;
	}
	return fd1 == AT_FDCWD || *path1 == '/' ? fd2 == AT_FDCWD || *path2 == '/' ? ::link(path1, path2) : _at(fd2, ::link(path1, path2)) : _at(fd1, ::link(path1, path2));
}

int mkdirat(int fd, const char *path, mode_t mode) {
	return fd == AT_FDCWD || *path == '/' ? ::mkdir(path, mode) : _at(fd, ::mkdir(path, mode));
}

int mkfifoat(int fd, const char *path, mode_t mode) {
	return fd == AT_FDCWD || *path == '/' ? ::mkfifo(path, mode) : _at(fd, ::mkfifo(path, mode));
}

int mknodat(int fd, const char *path, mode_t mode, dev_t dev) {
	return fd == AT_FDCWD || *path == '/' ? ::mknod(path, mode, dev) : _at(fd, ::mknod(path, mode, dev));
}

int openat(int fd, const char *path, int oflag, ...) {
	if (oflag & O_CREAT) {
		va_list ap;
		va_start(ap, oflag);
		unsigned mode = va_arg(ap, unsigned);
		va_end(ap);
		return fd == AT_FDCWD || *path == '/' ? ::open(path, oflag, mode) : _at(fd, ::open(path, oflag, mode));
	}
	return fd == AT_FDCWD || *path == '/' ? ::open(path, oflag) : _at(fd, ::open(path, oflag));
}

ssize_t readlinkat(int fd, const char * _restrict path, char * _restrict buf, size_t bufsize) {
	return fd == AT_FDCWD || *path == '/' ? ::readlink(path, buf, bufsize) : _at(fd, ::readlink(path, buf, bufsize));
}

int renameat(int oldfd, const char *oldpath, int newfd, const char *newpath) {
	if (oldfd != newfd && *oldpath != '/' && *newpath != '/') {
		errno = ENOTSUP;
		return -1;
	}
	return oldfd == AT_FDCWD || *oldpath == '/' ? newfd == AT_FDCWD || *newpath == '/' ? ::link(oldpath, newpath) : _at(newfd, ::link(oldpath, newpath)) : _at(oldfd, ::link(oldpath, newpath));
}

int symlinkat(const char *path1, int fd, const char *path2) {
	return fd == AT_FDCWD || *path2 == '/' ? ::symlink(path1, path2) : _at(fd, ::symlink(path1, path2));
}

int unlinkat(int fd, const char *path, int flag) {
	if (flag != 0) {
		if (flag == AT_REMOVEDIR) {
			return fd == AT_FDCWD || *path == '/' ? ::rmdir(path) : _at(fd, ::rmdir(path));
		}
		errno = EINVAL;
		return -1;
	}
	return fd == AT_FDCWD || *path == '/' ? ::unlink(path) : _at(fd, ::unlink(path));
}

int utimensat(int fd, const char *path, const struct timespec times[2], int flag) {
	struct timeval tvs[2], *ptvs = tvs;
	if (::utimens_to_utimes(&ptvs, times) < 0) {
		return -1;
	}
	if (flag != 0) {
		if (flag == AT_SYMLINK_NOFOLLOW) {
			return fd == AT_FDCWD || *path == '/' ? ::lutimes(path, ptvs) : _at(fd, ::lutimes(path, ptvs));
		}
		errno = EINVAL;
		return -1;
	}
	return fd == AT_FDCWD || *path == '/' ? ::utimes(path, ptvs) : _at(fd, ::utimes(path, ptvs));
}

int posix_fadvise(int fd, off_t offset, off_t len, int advice) {
	switch (advice) {
		case POSIX_FADV_NORMAL:
		case POSIX_FADV_SEQUENTIAL:
			return ::fcntl(fd, F_RDAHEAD, 1) == 0 ? 0 : errno;
		case POSIX_FADV_RANDOM:
			return ::fcntl(fd, F_RDAHEAD, 0) == 0 ? 0 : errno;
		case POSIX_FADV_WILLNEED: {
			struct radvisory ra;
			ra.ra_offset = offset;
			ra.ra_count = saturate<decltype(ra.ra_count)>(len);
			return ::fcntl(fd, F_RDADVISE, &ra) == 0 ? 0 : errno;
		}
	}
	return 0;
}

int _const posix_fallocate(int, off_t, off_t) {
	return ENOSYS;
}

ssize_t preadv(int fd, const struct iovec iov[], int iovcnt, off_t offset) {
	if (iovcnt <= 0) {
		if (iovcnt == 0) {
			return 0;
		}
		errno = EINVAL;
		return -1;
	}
	size_t size = 0;
	void *one_buf = nullptr;
	for (int i = iovcnt; --i >= 0;) {
		if (iov[i].iov_len != 0) {
			one_buf = size == 0 ? iov[i].iov_base : nullptr;
		}
		if (static_cast<ssize_t>(iov[i].iov_len) < 0 || static_cast<ssize_t>(size += iov[i].iov_len) < 0) {
			errno = EINVAL;
			return -1;
		}
	}
	if (size == 0) {
		return 0;
	}
	if (one_buf) {
		return ::pread(fd, one_buf, size, offset);
	}
	auto buf = make_buffer(size);
	ssize_t ret = ::pread(fd, buf.get(), size, offset);
	if (ret > 0) {
		const uint8_t *p = buf.get();
		size_t r = ret;
		do {
			size_t n = std::min(r, iov->iov_len);
			std::memcpy(iov->iov_base, p, n);
			++iov, p += n, r -= n;
		} while (r > 0);
	}
	return ret;
}

ssize_t pwritev(int fd, const struct iovec iov[], int iovcnt, off_t offset) {
	if (iovcnt <= 0) {
		if (iovcnt == 0) {
			return 0;
		}
		errno = EINVAL;
		return -1;
	}
	size_t size = 0;
	const void *one_buf = nullptr;
	for (int i = iovcnt; --i >= 0;) {
		if (iov[i].iov_len != 0) {
			one_buf = size == 0 ? iov[i].iov_base : nullptr;
		}
		if (static_cast<ssize_t>(iov[i].iov_len) < 0 || static_cast<ssize_t>(size += iov[i].iov_len) < 0) {
			errno = EINVAL;
			return -1;
		}
	}
	if (size == 0) {
		return 0;
	}
	if (one_buf) {
		return ::pwrite(fd, one_buf, size, offset);
	}
	auto buf = make_buffer(size);
	uint8_t *p = buf.get();
	for (int i = 0; i < iovcnt; ++i) {
		std::memcpy(p, iov[i].iov_base, iov[i].iov_len);
		p += iov[i].iov_len;
	}
	return ::pwrite(fd, buf.get(), size, offset);
}

#endif // defined(__APPLE__)

#endif // _POSIX_VERSION < 200809L
