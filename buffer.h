#pragma once

#include <array>
#include <cstdlib>
#include <cstring>
#include <new>

#include "bit.h"
#include "compiler.h"
#include "span.h"


template <typename T>
struct BasicBufferView {
	T *bptr, *gptr, *pptr, *eptr;
	constexpr BasicBufferView() noexcept : bptr(), gptr(), pptr(), eptr() { }
	constexpr BasicBufferView(T *bptr, T *gptr, T *pptr, T *eptr) noexcept : bptr(bptr), gptr(gptr), pptr(pptr), eptr(eptr) { }
	constexpr BasicBufferView(T *bptr, T *eptr) noexcept : BasicBufferView(bptr, bptr, bptr, eptr) { }
	constexpr BasicBufferView(T *bptr, size_t size) noexcept : BasicBufferView(bptr, bptr + size) { }
	constexpr BasicBufferView(std::span<T> span) noexcept : BasicBufferView(span.begin(), span.end()) { }
	constexpr _const operator BasicBufferView<const T> & () noexcept { return reinterpret_cast<BasicBufferView<const T> &>(*this); }
	constexpr _const operator const BasicBufferView<const T> & () const noexcept { return reinterpret_cast<const BasicBufferView<const T> &>(*this); }
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

using BufferView = BasicBufferView<std::byte>;
using ConstBufferView = BasicBufferView<const std::byte>;


template <typename T, size_t N>
struct BasicStaticBuffer : BasicBufferView<T>, std::array<T, N> {
	constexpr BasicStaticBuffer() noexcept : BasicBufferView<T>(this->begin(), this->end()) { }
private:
	BasicStaticBuffer(const BasicStaticBuffer &) = delete;
	BasicStaticBuffer & operator=(const BasicStaticBuffer &) = delete;
};

template <size_t N>
using StaticBuffer = BasicStaticBuffer<std::byte, N>;


template <typename T>
struct BasicDynamicBuffer : BasicBufferView<T> {
	BasicDynamicBuffer() noexcept = default;
	explicit BasicDynamicBuffer(size_t size) : BasicBufferView<T>(static_cast<T *>(size == 0 ? nullptr : sizeof(T) == 1 ? std::malloc(size) : reallocarray(nullptr, size, sizeof(T))), size) { if (_unlikely(!this->bptr && size)) throw std::bad_alloc(); }
	BasicDynamicBuffer(BasicDynamicBuffer &&move) noexcept : BasicBufferView<T>(std::move(move)) { move.eptr = move.pptr = move.gptr = move.bptr = nullptr; }
	BasicDynamicBuffer & operator=(BasicDynamicBuffer &&move) noexcept { return this->swap(move), *this; }
	~BasicDynamicBuffer() noexcept { std::free(this->bptr); }
	void swap(BasicDynamicBuffer &other) noexcept { using std::swap; swap(this->bptr, other.bptr), swap(this->gptr, other.gptr), swap(this->pptr, other.pptr), swap(this->eptr, other.eptr); }
	friend void swap(BasicDynamicBuffer &lhs, BasicDynamicBuffer &rhs) noexcept { lhs.swap(rhs); }
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
			this->resize(std::bit_ceil(min_size));
		}
	}
	void append(std::span<const T> data) {
		if (this->pptr + data.size() > this->eptr) {
			this->resize(std::bit_ceil(this->ppos() + data.size()));
		}
		std::memcpy(this->pptr, data.data(), data.size_bytes()), this->pptr += data.size();
	}
	_deprecated void append(const T data[], size_t n) {
		return this->append({ data, n });
	}
	std::enable_if_t<sizeof(T) == 1, void> append(const void *data, size_t n) {
		return this->append({ static_cast<const T *>(data), n });
	}
private:
	BasicDynamicBuffer(const BasicDynamicBuffer &) = delete;
	BasicDynamicBuffer & operator=(const BasicDynamicBuffer &) = delete;
#if !__GLIBC__ || !__GLIBC_PREREQ(2, 26)
	static inline void * reallocarray(void *ptr, size_t nmemb, size_t size) noexcept {
		return size == 0 || nmemb <= SIZE_MAX / size ? std::realloc(ptr, nmemb * size) : nullptr;
	}
#endif
};

using DynamicBuffer = BasicDynamicBuffer<std::byte>;
