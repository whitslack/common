#pragma once

#include <bit>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <new>

#include "compiler.h"


template <typename T>
struct BasicStaticBuffer {
	T *bptr, *gptr, *pptr, *eptr;
	constexpr BasicStaticBuffer() noexcept : bptr(), gptr(), pptr(), eptr() { }
	constexpr BasicStaticBuffer(T *bptr, T *gptr, T *pptr, T *eptr) noexcept : bptr(bptr), gptr(gptr), pptr(pptr), eptr(eptr) { }
	constexpr BasicStaticBuffer(T *bptr, T *eptr) noexcept : BasicStaticBuffer(bptr, bptr, bptr, eptr) { }
	constexpr BasicStaticBuffer(T *bptr, size_t size) noexcept : BasicStaticBuffer(bptr, bptr + size) { }
	constexpr size_t _pure gpos() const noexcept { return gptr - bptr; }
	constexpr void gpos(size_t gpos) noexcept { gptr = bptr + gpos; }
	constexpr size_t _pure ppos() const noexcept { return pptr - bptr; }
	constexpr void ppos(size_t ppos) noexcept { pptr = bptr + ppos; }
	constexpr size_t _pure grem() const noexcept { return pptr - gptr; }
	constexpr size_t _pure prem() const noexcept { return eptr - pptr; }
	constexpr size_t _pure size() const noexcept { return eptr - bptr; }
	constexpr void clear() noexcept { pptr = gptr = bptr; }
	void compact() noexcept { if (size_t gpos = this->gpos()) std::memmove(bptr, gptr, this->grem() * sizeof(T)), pptr -= gpos, gptr = bptr; }
};

using StaticBuffer = BasicStaticBuffer<uint8_t>;


template <typename T>
struct BasicBuffer : BasicStaticBuffer<T> {
	BasicBuffer() noexcept = default;
	explicit BasicBuffer(size_t size) : BasicStaticBuffer<T>(static_cast<T *>(size == 0 ? nullptr : sizeof(T) == 1 ? std::malloc(size) : reallocarray(nullptr, size, sizeof(T))), size) { if (_unlikely(!this->bptr && size)) throw std::bad_alloc(); }
	BasicBuffer(BasicBuffer &&move) noexcept : BasicStaticBuffer<T>(std::move(move)) { move.eptr = move.pptr = move.gptr = move.bptr = nullptr; }
	BasicBuffer & operator = (BasicBuffer &&move) noexcept { return this->swap(move), *this; }
	~BasicBuffer() noexcept { std::free(this->bptr); }
	void swap(BasicBuffer &other) noexcept { using std::swap; swap(this->bptr, other.bptr), swap(this->gptr, other.gptr), swap(this->pptr, other.pptr), swap(this->eptr, other.eptr); }
	friend void swap(BasicBuffer &lhs, BasicBuffer &rhs) noexcept { lhs.swap(rhs); }
	void resize(size_t size) {
		T *new_bptr = static_cast<T *>(sizeof(T) == 1 ? std::realloc(this->bptr, size) : reallocarray(this->bptr, size, sizeof(T)));
		if (_unlikely(!new_bptr && size)) {
			throw std::bad_alloc();
		}
		size_t gpos = this->gpos(), ppos = this->ppos();
		this->eptr = (this->bptr = new_bptr) + size;
		this->gpos(gpos), this->ppos(ppos);
	}
	void ensure(size_t min_size) {
		if (this->size() < min_size) {
			this->resize(std::ceil2(min_size));
		}
	}
	void append(const T data[], size_t n) {
		if (this->pptr + n > this->eptr) {
			this->resize(std::ceil2(this->ppos() + n));
		}
		std::memcpy(this->pptr, data, n * sizeof(T)), this->pptr += n;
	}
	std::enable_if_t<sizeof(T) == 1, void> append(const void *data, size_t n) {
		return this->append(static_cast<const T *>(data), n);
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
