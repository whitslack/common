#include "sockaddr.h"

#include <cstdlib>
#include <istream>
#include <new>
#include <ostream>
#include <system_error>

#include <arpa/inet.h>


struct sockaddr * SocketAddress::alloc(socklen_t size) {
	if (u.size == size) {
		return _likely(size <= sizeof u.small.u) ? &u.small.u.sa : u.large.ptr;
	}
	if (_unlikely(u.size > sizeof u.small.u)) {
		std::free(u.large.ptr);
	}
	if (_likely(size <= sizeof u.small.u)) {
		u.small.size = size;
		return &u.small.u.sa;
	}
	if (!(u.large.ptr = static_cast<struct sockaddr *>(std::malloc(size)))) {
		u.size = 0;
		throw std::bad_alloc();
	}
	u.large.size = size;
	return u.large.ptr;
}


std::ostream & operator << (std::ostream &os, const struct in_addr &addr) {
	char buf[INET_ADDRSTRLEN];
	if (_unlikely(!::inet_ntop(AF_INET, &addr, buf, sizeof buf))) {
		throw std::system_error(errno, std::system_category(), "inet_ntop");
	}
	return os << buf;
}

std::ostream & operator << (std::ostream &os, const struct in6_addr &addr) {
	char buf[INET6_ADDRSTRLEN];
	if (_unlikely(!::inet_ntop(AF_INET6, &addr, buf, sizeof buf))) {
		throw std::system_error(errno, std::system_category(), "inet_ntop");
	}
	return os << buf;
}

std::istream & operator >> (std::istream &is, struct in_addr &addr) {
	if (std::string s; is >> s) {
		if (auto r = ::inet_pton(AF_INET, s.c_str(), &addr); _unlikely(r <= 0)) {
			if (_unlikely(r < 0)) {
				throw std::system_error(errno, std::system_category(), "inet_pton");
			}
			is.setstate(std::ios_base::failbit);
		}
	}
	return is;
}

std::istream & operator >> (std::istream &is, struct in6_addr &addr) {
	if (std::string s; is >> s) {
		if (auto r = ::inet_pton(AF_INET6, s.c_str(), &addr); _unlikely(r <= 0)) {
			if (_unlikely(r < 0)) {
				throw std::system_error(errno, std::system_category(), "inet_pton");
			}
			is.setstate(std::ios_base::failbit);
		}
	}
	return is;
}
