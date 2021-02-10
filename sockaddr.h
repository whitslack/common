#pragma once

#include <cstring>
#include <functional>
#include <iosfwd>

#include <netinet/in.h>

#include "compiler.h"
#include "endian.h"


#define _(OP, NOP) \
	static constexpr bool _pure operator OP (const struct in_addr &lhs, const struct in_addr &rhs) noexcept { return as_be(lhs.s_addr) OP as_be(rhs.s_addr); } \
	_N(struct in_addr, OP, NOP) \
	__(struct in6_addr, OP, NOP) \
	__(struct sockaddr_in, OP, NOP) \
	__(struct sockaddr_in6, OP, NOP)
#define __(T, OP, NOP) \
	static constexpr bool _pure operator OP (const T &lhs, const T &rhs) noexcept { return std::memcmp(&lhs, &rhs, sizeof(T)) OP 0; } \
	_N(T, OP, NOP)
#define _N(T, OP, NOP) \
	static constexpr bool _pure operator NOP (const T &lhs, const T &rhs) noexcept { return !(lhs OP rhs); }
_(==, !=)
_(<, >=)
_(>, <=)
#undef _N
#undef __
#undef _


class SocketAddress {

private:
	union {
		socklen_t size;
		struct {
			socklen_t size;
			union {
				struct sockaddr sa;
				struct sockaddr_in sin;
				struct sockaddr_in6 sin6;
			} u;
		} small;
		struct {
			socklen_t size;
			struct sockaddr *ptr;
		} large;
	} u;

public:
	constexpr SocketAddress() noexcept : u { } { }
	SocketAddress(const struct sockaddr *address, socklen_t size) : SocketAddress() { this->assign(address, size); }
	constexpr SocketAddress(const struct sockaddr_in &sin) noexcept : u { } { u.size = sizeof sin; u.small.u.sin = sin; }
	constexpr SocketAddress(const struct sockaddr_in6 &sin6) noexcept : u { } { u.size = sizeof sin6; u.small.u.sin6 = sin6; }
	SocketAddress(const SocketAddress &copy) : SocketAddress(copy, copy.size()) { }
	SocketAddress & operator=(const SocketAddress &copy) { return this->assign(copy, copy.size()), *this; }
	SocketAddress(SocketAddress &&move) noexcept : SocketAddress() { this->swap(move); }
	SocketAddress & operator=(SocketAddress &&move) noexcept { return this->swap(move), *this; }
	void swap(SocketAddress &other) noexcept { using std::swap; swap(u, other.u); }
	friend void swap(SocketAddress &lhs, SocketAddress &rhs) noexcept { lhs.swap(rhs); }
	~SocketAddress() { if (_unlikely(u.size > sizeof u.small.u)) std::free(u.large.ptr); }

public:
	constexpr explicit _pure operator bool () const noexcept { return u.size; }
	constexpr socklen_t _pure size() const noexcept { return u.size; }
	constexpr sa_family_t _pure family() const noexcept { return (**this).sa_family; }

	constexpr _pure operator const struct sockaddr * () const noexcept { return _likely(u.size <= sizeof u.small.u) ? &u.small.u.sa : u.large.ptr; }
	constexpr explicit _const operator const struct sockaddr_in & () const noexcept { return u.small.u.sin; }
	constexpr explicit _const operator const struct sockaddr_in6 & () const noexcept { return u.small.u.sin6; }

	struct sockaddr * alloc(socklen_t size);
	void assign(const struct sockaddr *address, socklen_t size) { std::memcpy(this->alloc(size), address, size); }

	friend constexpr bool _pure operator==(const SocketAddress &lhs, const SocketAddress &rhs) noexcept { return lhs.u.size == rhs.u.size && std::memcmp(lhs, rhs, lhs.u.size) == 0; }
	friend constexpr bool _pure operator<(const SocketAddress &lhs, const SocketAddress &rhs) noexcept { return lhs.u.size < rhs.u.size || lhs.u.size == rhs.u.size && std::memcmp(lhs, rhs, lhs.u.size) < 0; }
	friend constexpr bool _pure operator>(const SocketAddress &lhs, const SocketAddress &rhs) noexcept { return lhs.u.size > rhs.u.size || lhs.u.size == rhs.u.size && std::memcmp(lhs, rhs, lhs.u.size) > 0; }

#define _N(LT, RT) \
	_N_(LT, RT, ==, !=) \
	_N_(LT, RT, <, >=) \
	_N_(LT, RT, >, <=)
#define _N_(LT, RT, OP, NOP) \
	friend constexpr bool _pure operator NOP (const LT &lhs, const RT &rhs) noexcept { return !(lhs OP rhs); }
_N(SocketAddress, SocketAddress)

#define _(T, M) \
	friend constexpr bool _pure operator==(const SocketAddress &lhs, const T &rhs) noexcept { return lhs.u.size == sizeof(T) && lhs.u.small.u.M == rhs; } \
	_R(T, ==, ==) \
	__(T, M, <, >) \
	__(T, M, >, <) \
	_N(SocketAddress, T) \
	_N(T, SocketAddress)
#define __(T, M, OP, ROP) \
	friend constexpr bool _pure operator OP (const SocketAddress &lhs, const T &rhs) noexcept { return lhs.u.size OP sizeof(T) || lhs.u.size == sizeof(T) && lhs.u.small.u.M OP rhs; } \
	_R(T, OP, ROP)
#define _R(T, OP, ROP) \
	friend constexpr bool _pure operator OP (const T &lhs, const SocketAddress &rhs) noexcept { return rhs ROP lhs; }
_(struct sockaddr_in, sin)
_(struct sockaddr_in6, sin6)
#undef _R
#undef __
#undef _

#undef _N_
#undef _N

};


#define IN_IS_ADDR_UNSPECIFIED(a) ((const struct in_addr *){a}->s_addr == 0)
#define IN_IS_ADDR_LOOPBACK(a) (((const struct in_addr *){a}->s_addr & htonl(0xff000000)) == htonl(0x7f000000))
#define IN_IS_ADDR_LINKLOCAL(a) (((const struct in_addr *){a}->s_addr & htonl(0xffff0000)) == htonl(0xa9fe0000))
#define IN_IS_ADDR_PRIVATE(a) (((const struct in_addr *){a}->s_addr & htonl(0xff000000)) == htonl(0x0a000000) || \
		((const struct in_addr *){a}->s_addr & htonl(0xfff00000)) == htonl(0xac100000) || \
		((const struct in_addr *){a}->s_addr & htonl(0xffff0000)) == htonl(0xc0a80000))


template <>
struct std::hash<struct in_addr> {
	constexpr size_t _pure operator()(const struct in_addr &addr) const noexcept {
		return as_be(addr.s_addr);
	}
};

template <>
struct std::hash<struct in6_addr> {
	constexpr size_t _pure operator()(const struct in6_addr &addr) const noexcept {
		size_t hash { }, len = sizeof addr.s6_addr;
		for (auto ptr = reinterpret_cast<const std::byte *>(addr.s6_addr); len >= sizeof hash; ptr += sizeof hash, len -= sizeof hash) {
			hash ^= *reinterpret_cast<const size_t *>(ptr);
		}
		return as_be(hash);
	}
};

template <>
struct std::hash<struct sockaddr_in> {
	constexpr size_t _pure operator()(const struct sockaddr_in &addr) const noexcept {
		return hash<struct in_addr>()(addr.sin_addr) ^ as_be(addr.sin_port);
	}
};

template <>
struct std::hash<struct sockaddr_in6> {
	constexpr size_t _pure operator()(const struct sockaddr_in6 &addr) const noexcept {
		return hash<struct in6_addr>()(addr.sin6_addr) ^ as_be(addr.sin6_port);
	}
};


std::string _pure to_string(const struct in_addr &addr);
std::string _pure to_string(const struct in6_addr &addr);

std::ostream & operator<<(std::ostream &os, const struct in_addr &addr);
std::ostream & operator<<(std::ostream &os, const struct in6_addr &addr);
std::istream & operator>>(std::istream &is, struct in_addr &addr);
std::istream & operator>>(std::istream &is, struct in6_addr &addr);
