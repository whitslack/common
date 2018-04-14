#pragma once

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <new>

#include "compiler.h"

template <typename T>
struct BasicBuffer {
	T *base, *gptr, *pptr, *end;
	BasicBuffer() noexcept : base(), gptr(), pptr(), end() { }
	BasicBuffer(T *base, T *gptr, T *pptr, T *end) noexcept : base(base), gptr(gptr), pptr(pptr), end(end) { }
	BasicBuffer(T *base, T *end) noexcept : BasicBuffer(base, base, base, end) { }
	BasicBuffer(T *base, size_t size) noexcept : BasicBuffer(base, base + size) { }
	explicit BasicBuffer(size_t size) : BasicBuffer(static_cast<T *>(size == 0 ? nullptr : sizeof(T) == 1 ? std::malloc(size) : reallocarray(nullptr, size, sizeof(T))), size) { if (!base && size) throw std::bad_alloc(); }
	BasicBuffer(BasicBuffer &&move) noexcept : base(move.base), gptr(move.gptr), pptr(move.pptr), end(move.end) { move.end = move.pptr = move.gptr = move.base = nullptr; }
	BasicBuffer & operator = (BasicBuffer &&move) noexcept { return this->swap(move), *this; }
	~BasicBuffer() noexcept { std::free(base); }
	void swap(BasicBuffer &other) noexcept { using std::swap; swap(base, other.base), swap(gptr, other.gptr), swap(pptr, other.pptr), swap(end, other.end); }
	friend void swap(BasicBuffer &lhs, BasicBuffer &rhs) noexcept { lhs.swap(rhs); }
	size_t _pure gpos() const noexcept { return gptr - base; }
	void gpos(size_t gpos) noexcept { gptr = base + gpos; }
	size_t _pure ppos() const noexcept { return pptr - base; }
	void ppos(size_t ppos) noexcept { pptr = base + ppos; }
	size_t _pure grem() const noexcept { return pptr - gptr; }
	size_t _pure prem() const noexcept { return end - pptr; }
	size_t _pure size() const noexcept { return end - base; }
	void clear() noexcept { pptr = gptr = base; }
	void compact() noexcept { if (size_t gpos = this->gpos()) std::memmove(base, gptr, this->grem() * sizeof(T)), pptr -= gpos, gptr = base; }
	void resize(size_t size) {
		T *new_base = static_cast<T *>(sizeof(T) == 1 ? std::realloc(base, size) : reallocarray(base, size, sizeof(T)));
		if (!new_base && size) {
			throw std::bad_alloc();
		}
		size_t gpos = this->gpos(), ppos = this->ppos();
		end = (base = new_base) + size;
		this->gpos(gpos), this->ppos(ppos);
	}
	void ensure(size_t min_size) {
		if (this->size() < min_size) {
			this->resize(size_t(1) << SIZE_WIDTH - _clz(min_size - 1));
		}
	}
	void append(const void *data, size_t n) {
		if (pptr + n > end) {
			this->resize(size_t(1) << SIZE_WIDTH - _clz(this->ppos() + n));
		}
		std::memcpy(pptr, data, n), pptr += n;
	}
private:
	BasicBuffer(const BasicBuffer &) = delete;
	BasicBuffer & operator = (const BasicBuffer &) = delete;
#if !__GLIBC__ || !__GLIBC_PREREQ(2, 26)
	static inline void * reallocarray(void *ptr, size_t nmemb, size_t size) noexcept {
		return size == 0 || nmemb <= SIZE_MAX / size ? std::realloc(ptr, nmemb * size) : nullptr;
	}
#endif
};

using Buffer = BasicBuffer<uint8_t>;
