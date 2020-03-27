#include "dns.h"

#include <cstring>
#include <ostream>


const char * GAICategory::name() const noexcept {
	return "GAI";
}

std::string GAICategory::message(int condition) const {
	return ::gai_strerror(condition);
}


GAIResults getaddrinfo(const char host[], const char service[], int family, int type, int protocol, int flags) {
	addrinfo hints;
	std::memset(&hints, 0, sizeof hints);
	hints.ai_family = family;
	hints.ai_socktype = type;
	hints.ai_protocol = protocol;
	hints.ai_flags = flags;
	addrinfo *res;
	if (int error = ::getaddrinfo(host, service, &hints, &res); _unlikely(error != 0)) {
		if (error == EAI_SYSTEM) {
			throw std::system_error(errno, std::system_category(), "getaddrinfo");
		}
		else {
			throw std::system_error(error, GAICategory(), "getaddrinfo");
		}
	}
	return GAIResults(res);
}


std::ostream & operator << (std::ostream &os, const struct sockaddr &addr) {
	switch (addr.sa_family) {
		case AF_INET:
			return os << reinterpret_cast<const struct sockaddr_in &>(addr);
		case AF_INET6:
			return os << reinterpret_cast<const struct sockaddr_in6 &>(addr);
		default:
			return os << "(unsupported address)";
	}
}

std::ostream & operator << (std::ostream &os, const struct sockaddr_in &addr) {
	char host[NI_MAXHOST], serv[NI_MAXSERV];
	if (int error = ::getnameinfo(reinterpret_cast<const sockaddr *>(&addr), static_cast<socklen_t>(sizeof addr), host, static_cast<socklen_t>(sizeof host), serv, static_cast<socklen_t>(sizeof serv), NI_NUMERICHOST | NI_NUMERICSERV); _unlikely(error != 0)) {
		if (error == EAI_SYSTEM) {
			throw std::system_error(errno, std::system_category(), "getnameinfo");
		}
		else {
			throw std::system_error(error, GAICategory(), "getnameinfo");
		}
	}
	return os << host << ':' << serv;
}

std::ostream & operator << (std::ostream &os, const struct sockaddr_in6 &addr) {
	char host[NI_MAXHOST], serv[NI_MAXSERV];
	if (int error = ::getnameinfo(reinterpret_cast<const sockaddr *>(&addr), static_cast<socklen_t>(sizeof addr), host, static_cast<socklen_t>(sizeof host), serv, static_cast<socklen_t>(sizeof serv), NI_NUMERICHOST | NI_NUMERICSERV); _unlikely(error != 0)) {
		if (error == EAI_SYSTEM) {
			throw std::system_error(errno, std::system_category(), "getnameinfo");
		}
		else {
			throw std::system_error(error, GAICategory(), "getnameinfo");
		}
	}
	return os << '[' << host << "]:" << serv;
}
