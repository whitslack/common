#include "dns.h"

#include <cassert>
#include <cstring>
#include <ostream>
#include <string>

#include "endian.h"


const char * GAICategory::name() const noexcept {
	return "GAI";
}

std::string GAICategory::message(int condition) const {
	return ::gai_strerror(condition);
}


GAIResults getaddrinfo(const char host[], const char service[], int family, int type, int protocol, int flags) {
	struct addrinfo hints { };
	hints.ai_family = family;
	hints.ai_socktype = type;
	hints.ai_protocol = protocol;
	hints.ai_flags = flags;
	struct addrinfo *res;
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

GAIResults getaddrinfo(const char host_and_service[], in_port_t default_port, int family, int type, int protocol, int flags) {
	std::string host_;
	const char *host, *service;
	if (*host_and_service == '[' && (service = std::strchr(host_and_service, ']'))) {
		host = host_.assign(host_and_service + 1, service++).c_str();
		if (*service == ':') {
			++service;
		}
		else if (*service == '\0') {
			service = nullptr;
		}
		else {
			throw std::invalid_argument(host_and_service);
		}
	}
	else if (service = std::strrchr(host_and_service, ':')) {
		host = host_.assign(host_and_service, service++).c_str();
	}
	else {
		host = host_and_service;
	}
	GAIResults results = getaddrinfo(host, service, family, type, protocol, flags);
	if (!service && default_port != 0) {
		for (auto &each : results) {
			if (each.ai_family == AF_INET6) {
				auto &sin6 = *reinterpret_cast<struct sockaddr_in6 *>(each.ai_addr);
				if (each.ai_addrlen == sizeof sin6 && sin6.sin6_port == 0) {
					as_be(sin6.sin6_port) = default_port;
				}
			}
			else if (each.ai_family == AF_INET) {
				auto &sin = *reinterpret_cast<struct sockaddr_in *>(each.ai_addr);
				if (each.ai_addrlen == sizeof sin && sin.sin_port == 0) {
					as_be(sin.sin_port) = default_port;
				}
			}
		}
	}
	return results;
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
	assert(addr.sin_family == AF_INET);
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
	assert(addr.sin6_family == AF_INET6);
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
