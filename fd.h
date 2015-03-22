#pragma once

#include <fcntl.h>
#include <poll.h>
#include <sys/mman.h>

#include "io.h"


namespace posix {

void access(const char *path, int amode);
void chdir(const char *path);
void chmod(const char *path, mode_t mode);
void chown(const char *path, uid_t owner, gid_t group);
void close(int fildes);
int creat(const char *path, mode_t mode = 0666);
int dup(int fildes);
int dup2(int fildes, int fildes2);
void faccessat(int fd, const char *path, int amode, int flag = 0);
void fadvise(int fd, off_t offset, off_t len, int advice);
void fallocate(int fd, off_t offset, off_t len);
void fchdir(int fildes);
void fchmod(int fildes, mode_t mode);
void fchmodat(int fd, const char *path, mode_t mode, int flag = 0);
void fchown(int fildes, uid_t owner, gid_t group);
void fchownat(int fd, const char *path, uid_t owner, gid_t group, int flag = 0);
int fcntl(int fildes, int cmd);
int fcntl(int fildes, int cmd, int arg);
int fcntl(int fildes, int cmd, void *arg);
void fdatasync(int fildes);
void fstat(int fildes, struct stat *buf);
void fstatat(int fd, const char * _restrict path, struct stat * _restrict buf, int flag = 0);
void fsync(int fildes);
void ftruncate(int fildes, off_t length = 0);
void futimens(int fd, const struct timespec times[2]);
int ioctl(int fildes, int request);
int ioctl(int fildes, int request, int arg);
int ioctl(int fildes, int request, void *arg);
bool isatty(int fildes);
void lchown(const char *path, uid_t owner, gid_t group);
void link(const char *path1, const char *path2);
void linkat(int fd1, const char *path1, int fd2, const char *path2, int flag = 0);
void lockf(int fildes, int function, off_t size);
off_t lseek(int fildes, off_t offset, int whence = SEEK_SET);
void lstat(const char * _restrict path, struct stat * _restrict buf);
void madvise(void *addr, size_t len, int advice);
void mkdir(const char *path, mode_t mode = 0777);
void mkdirat(int fd, const char *path, mode_t mode = 0777);
void mkfifo(const char *path, mode_t mode = 0666);
void mkfifoat(int fd, const char *path, mode_t mode = 0666);
void mknod(const char *path, mode_t mode, dev_t dev);
void mknodat(int fd, const char *path, mode_t mode, dev_t dev);
void * mmap(void *addr, size_t len, int prot, int flags, int fildes, off_t off);
void mprotect(void *addr, size_t len, int prot);
void msync(void *addr, size_t len, int flags);
void munmap(void *addr, size_t len);
int open(const char *path, int oflag, mode_t mode = 0666);
int openat(int fd, const char *path, int oflag, mode_t mode = 0666);
void pipe(int fildes[2]);
int poll(struct pollfd fds[], nfds_t nfds, int timeout = -1);
ssize_t pread(int fildes, void *buf, size_t nbyte, off_t offset);
size_t pwrite(int fildes, const void *buf, size_t nbyte, off_t offset);
ssize_t read(int fildes, void *buf, size_t nbyte);
size_t readlink(const char * _restrict path, char * _restrict buf, size_t bufsize);
size_t readlinkat(int fd, const char * _restrict path, char * _restrict buf, size_t bufsize);
void rmdir(const char *path);
void stat(const char * _restrict path, struct stat * _restrict buf);
void symlink(const char *path1, const char *path2);
void symlinkat(const char *path1, int fd, const char *path2);
void truncate(const char *path, off_t length = 0);
void unlink(const char *path);
void unlinkat(int fd, const char *path, int flag = 0);
void utimensat(int fd, const char *path, const struct timespec times[2], int flag = 0);
void utimes(const char *path, const struct timeval times[2]);
size_t write(int fildes, const void *buf, size_t nbyte);

} // namespace posix


class FileDescriptor : public Source, public Sink {

public:
	class MemoryMapping {
		friend FileDescriptor;
	private:
		void *addr;
		size_t len;
	public:
		MemoryMapping(MemoryMapping &&move) : addr(move.addr), len(move.len) { move.addr = MAP_FAILED; }
		MemoryMapping & operator = (MemoryMapping &&move) { std::swap(addr, move.addr); std::swap(len, move.len); return *this; }
		~MemoryMapping() { if (addr != MAP_FAILED) posix::munmap(addr, len); }
	private:
		MemoryMapping(void *addr, size_t len) : addr(addr), len(len) { }
		MemoryMapping(const MemoryMapping &) = delete;
		MemoryMapping & operator = (const MemoryMapping &) = delete;
	public:
		operator void * () const { return addr; }
		void * data() const { return addr; }
		size_t size() const { return len; }
		void madvise(size_t offset, size_t len, int advice) { posix::madvise(static_cast<uint8_t *>(addr) + offset, len, advice); }
		void mprotect(size_t offset, size_t len, int prot) { posix::mprotect(static_cast<uint8_t *>(addr) + offset, len, prot); }
		void msync(size_t offset, size_t len, int flags = MS_ASYNC) { posix::msync(static_cast<uint8_t *>(addr) + offset, len, flags); }
	};

protected:
	int fd;

public:
	FileDescriptor() : fd(-1) { }
	explicit FileDescriptor(int fd) : fd(fd) { }
	explicit FileDescriptor(const char *path, int oflag = O_RDONLY | O_CLOEXEC, mode_t mode = 0666) : fd(posix::open(path, oflag, mode)) { }
	FileDescriptor(FileDescriptor &&move) : fd(move.fd) { move.fd = -1; }
	FileDescriptor & operator = (FileDescriptor &&move) { std::swap(fd, move.fd); return *this; }
	virtual ~FileDescriptor() { if (fd >= 0) posix::close(fd); }

private:
	FileDescriptor(const FileDescriptor &) = delete;
	FileDescriptor & operator = (const FileDescriptor &) = delete;

public:
	operator int () const { return fd; }

	void creat(const char *path, mode_t mode = 0666) { *this = FileDescriptor(posix::creat(path, mode)); }
	void open(const char *path, int oflag = O_RDONLY | O_CLOEXEC, mode_t mode = 0666) { *this = FileDescriptor(path, oflag, mode); }
	void close() { posix::close(fd), fd = -1; }
	ssize_t read(void *buf, size_t n) override { return posix::read(fd, buf, n); }
	size_t write(const void *buf, size_t n) override { return posix::write(fd, buf, n); }
	ssize_t pread(void *buf, size_t nbyte, off_t offset) { return posix::pread(fd, buf, nbyte, offset); }
	size_t pwrite(const void *buf, size_t nbyte, off_t offset) { return posix::pwrite(fd, buf, nbyte, offset); }
	void lockf(int function, off_t size) { posix::lockf(fd, function, size); }
	int fcntl(int cmd) { return posix::fcntl(fd, cmd); }
	int fcntl(int cmd, int arg) { return posix::fcntl(fd, cmd, arg); }
	int fcntl(int cmd, void *arg) { return posix::fcntl(fd, cmd, arg); }
	int ioctl(int request) { return posix::ioctl(fd, request); }
	int ioctl(int request, int arg) { return posix::ioctl(fd, request, arg); }
	int ioctl(int request, void *arg) { return posix::ioctl(fd, request, arg); }
	bool isatty() const { return posix::isatty(fd); }
	off_t lseek(off_t offset, int whence = SEEK_SET) { return posix::lseek(fd, offset, whence); }
	void fstat(struct stat *buf) const { posix::fstat(fd, buf); }
	void fchdir() const { posix::fchdir(fd); }
	void fchmod(mode_t mode) { posix::fchmod(fd, mode); }
	void fchown(uid_t owner, gid_t group) { posix::fchown(fd, owner, group); }
	void fallocate(off_t offset, off_t len) { posix::fallocate(fd, offset, len); }
	void ftruncate(off_t length = 0) { posix::ftruncate(fd, length); }
	void futimens(const struct timespec times[2]) { posix::futimens(fd, times); }
	void fsync() { posix::fsync(fd); }
	void fdatasync() { posix::fdatasync(fd); }
	void fadvise(off_t offset, off_t length, int advice) { posix::fadvise(fd, offset, length, advice); }
	MemoryMapping mmap(off_t off, size_t len, int prot = PROT_READ, int flags = MAP_SHARED) { return { posix::mmap(nullptr, len, prot, flags, fd, off), len }; }

	void faccessat(const char *path, int amode, int flag = 0) const { posix::faccessat(fd, path, amode, flag); }
	void fchmodat(const char *path, mode_t mode, int flag = 0) const { posix::fchmodat(fd, path, mode, flag); }
	void fchownat(const char *path, uid_t owner, gid_t group, int flag = 0) const { posix::fchownat(fd, path, owner, group, flag); }
	void fstatat(const char * _restrict path, struct stat * _restrict buf, int flag = 0) const { posix::fstatat(fd, path, buf, flag); }
	void linkat(const char *path1, const char *path2, int flag = 0) const { posix::linkat(fd, path1, fd, path2, flag); }
	void mkdirat(const char *path, mode_t mode = 0777) const { posix::mkdirat(fd, path, mode); }
	void mkfifoat(const char *path, mode_t mode = 0666) const { posix::mkfifoat(fd, path, mode); }
	void mknodat(const char *path, mode_t mode, dev_t dev) const { posix::mknodat(fd, path, mode, dev); }
	FileDescriptor openat(const char *path, int oflag, mode_t mode = 0666) const { return FileDescriptor(posix::openat(fd, path, oflag, mode)); }
	size_t readlinkat(const char * _restrict path, char * _restrict buf, size_t bufsize) const { return posix::readlinkat(fd, path, buf, bufsize); }
	void symlinkat(const char *path1, const char *path2) const { posix::symlinkat(path1, fd, path2); }
	void unlinkat(const char *path, int flag = 0) const { posix::unlinkat(fd, path, flag); }
	void utimensat(const char *path, const struct timespec times[2], int flag = 0) const { posix::utimensat(fd, path, times, flag); }

};
