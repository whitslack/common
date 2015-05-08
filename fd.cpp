#include "fd.h"

#include <cstdio>
#include <system_error>

#include <unistd.h>
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

} // namespace posix
