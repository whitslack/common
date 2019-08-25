#include "io.h"

#include <ios>
#include <stdexcept>


template <typename Readable, typename... Args>
static ssize_t read(Readable &&readable, std::span<const BufferPointer> bufs, Args &&...args) {
	ssize_t ret = 0;
	for (auto [buf, n] : bufs) {
		while (n > 0) {
			ssize_t r = std::forward<Readable>(readable).read(buf, n, std::forward<Args>(args)...);
			if (r <= 0) {
				return ret == 0 ? r : ret;
			}
			ret += r;
			buf = static_cast<std::byte *>(buf) + r, n -= r;
		}
	}
	return ret;
}

template <typename Readable, typename... Args>
static void read_fully(Readable &&readable, void *buf, size_t n, Args &&...args) {
	while (n > 0) {
		ssize_t r = std::forward<Readable>(readable).read(buf, n, std::forward<Args>(args)...);
		if (_likely(r > 0)) {
			buf = static_cast<std::byte *>(buf) + r, n -= r;
		}
		else if (r < 0) {
			throw std::ios_base::failure("premature EOF");
		}
		else {
			throw std::logic_error("non-blocking read in blocking context");
		}
	}
}

template <typename Readable, typename... Args>
static void read_fully(Readable &&readable, std::span<const BufferPointer> bufs, Args &&...args) {
	while (!bufs.empty()) {
		ssize_t r = std::forward<Readable>(readable).read(bufs, std::forward<Args>(args)...);
		if (_likely(r > 0)) {
			while ((r -= bufs.front().size) > 0) {
				bufs = bufs.subspan(1);
			}
			if (r < 0) {
				std::forward<Readable>(readable).read_fully(static_cast<std::byte *>(bufs.front().ptr) + bufs.front().size + r, -r, std::forward<Args>(args)...);
			}
			bufs = bufs.subspan(1);
		}
		else if (r < 0) {
			throw std::ios_base::failure("premature EOF");
		}
		else {
			throw std::logic_error("non-blocking read in blocking context");
		}
	}
}

template <typename Writable, typename... Args>
static size_t write(Writable &&writable, std::span<const ConstBufferPointer> bufs, Args &&...args) {
	size_t ret = 0;
	for (auto [buf, n] : bufs) {
		while (n > 0) {
			size_t w = std::forward<Writable>(writable).write(buf, n, std::forward<Args>(args)...);
			if (w == 0) {
				return ret;
			}
			ret += w;
			buf = static_cast<const std::byte *>(buf) + w, n -= w;
		}
	}
	return ret;
}

template <typename Writable, typename... Args>
static void write_fully(Writable &&writable, const void *buf, size_t n, Args &&...args) {
	while (n > 0) {
		size_t w = std::forward<Writable>(writable).write(buf, n, std::forward<Args>(args)...);
		if (_likely(w > 0)) {
			buf = static_cast<const std::byte *>(buf) + w, n -= w;
		}
		else {
			throw std::logic_error("non-blocking write in blocking context");
		}
	}
}

template <typename Writable, typename... Args>
static void write_fully(Writable &&writable, std::span<const ConstBufferPointer> bufs, Args &&...args) {
	while (!bufs.empty()) {
		size_t w = std::forward<Writable>(writable).write(bufs, std::forward<Args>(args)...);
		if (_likely(w > 0)) {
			while (w > bufs.front().size) {
				w -= bufs.front().size;
				bufs = bufs.subspan(1);
			}
			if (w < bufs[0].size) {
				std::forward<Writable>(writable).write_fully(static_cast<const std::byte *>(bufs.front().ptr) + w, bufs.front().size - w, std::forward<Args>(args)...);
			}
			bufs = bufs.subspan(1);
		}
		else {
			throw std::logic_error("non-blocking write in blocking context");
		}
	}
}

template <typename Flushable, typename... Args>
static void flush_fully(Flushable &&flushable, Args &&...args) {
	if (_unlikely(!std::forward<Flushable>(flushable).flush(std::forward<Args>(args)...))) {
		throw std::logic_error("non-blocking write in blocking context");
	}
}


template <typename Derived, typename... Args>
_nodiscard ssize_t Readable<Derived, Args...>::read(std::span<const BufferPointer> bufs, Args &&...args) {
	return ::read<Derived &, Args...>(static_cast<Derived &>(*this), bufs, std::forward<Args>(args)...);
}

template <typename Derived, typename... Args>
void Readable<Derived, Args...>::read_fully(void *buf, size_t n, Args &&...args) {
	return ::read_fully<Derived &, Args...>(static_cast<Derived &>(*this), buf, n, std::forward<Args>(args)...);
}

template <typename Derived, typename... Args>
void Readable<Derived, Args...>::read_fully(std::span<const BufferPointer> bufs, Args &&...args) {
	return ::read_fully<Derived &, Args...>(static_cast<Derived &>(*this), bufs, std::forward<Args>(args)...);
}


template <typename Derived, typename... Args>
_nodiscard size_t Writable<Derived, Args...>::write(std::span<const ConstBufferPointer> bufs, Args &&...args) {
	return ::write<Derived &, Args...>(static_cast<Derived &>(*this), bufs, std::forward<Args>(args)...);
}

template <typename Derived, typename... Args>
void Writable<Derived, Args...>::write_fully(const void *buf, size_t n, Args &&...args) {
	return ::write_fully<Derived &, Args...>(static_cast<Derived &>(*this), buf, n, std::forward<Args>(args)...);
}

template <typename Derived, typename... Args>
void Writable<Derived, Args...>::write_fully(std::span<const ConstBufferPointer> bufs, Args &&...args) {
	return ::write_fully<Derived &, Args...>(static_cast<Derived &>(*this), bufs, std::forward<Args>(args)...);
}


template <typename Derived, typename... Args>
void Flushable<Derived, Args...>::flush_fully(Args &&...args) {
	return ::flush_fully<Derived &, Args...>(static_cast<Derived &>(*this), std::forward<Args>(args)...);
}
