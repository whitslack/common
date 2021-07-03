#include <system_error>

#include <netdb.h>

#include "compiler.h"
#include "sockaddr.h"


class GAICategory : public std::error_category {

public:
	const char * name() const noexcept override _const;
	std::string message(int condition) const override;

};


class GAIResults {
	friend GAIResults getaddrinfo(const char [], const char [], int, int, int, int);

public:
	class Iterator : public std::iterator<std::forward_iterator_tag, struct addrinfo, void> {
		friend class GAIResults;
	private:
		struct addrinfo *ptr;
	public:
		Iterator() noexcept : ptr() { }
		_pure operator struct addrinfo * () const noexcept { return ptr; }
		struct addrinfo * _pure operator->() const noexcept { return ptr; }
		Iterator & operator++() noexcept {
			ptr = ptr->ai_next;
			return *this;
		}
		Iterator operator++(int) noexcept {
			Iterator copy(*this);
			ptr = ptr->ai_next;
			return copy;
		}
	private:
		explicit Iterator(struct addrinfo *ptr) noexcept : ptr(ptr) { }
	};

private:
	struct addrinfo *res;

public:
	GAIResults(GAIResults &&move) noexcept : res(move.res) { move.res = nullptr; }
	~GAIResults() { ::freeaddrinfo(res); }

	Iterator _pure begin() const noexcept { return Iterator(res); }
	Iterator _const end() const noexcept { return Iterator(); }

private:
	explicit GAIResults(struct addrinfo *res) noexcept : res(res) { }
	GAIResults(const GAIResults &) = delete;
	GAIResults & operator=(const GAIResults &) = delete;

};

GAIResults getaddrinfo(const char host[], const char service[] = nullptr, int family = AF_UNSPEC, int type = SOCK_STREAM, int protocol = 0, int flags = AI_V4MAPPED | AI_ADDRCONFIG);
GAIResults getaddrinfo(const char host_and_service[], in_port_t default_port, int family = AF_UNSPEC, int type = SOCK_STREAM, int protocol = 0, int flags = AI_V4MAPPED | AI_ADDRCONFIG);


std::ostream & operator<<(std::ostream &os, const struct sockaddr &addr);
std::ostream & operator<<(std::ostream &os, const struct sockaddr_in &addr);
std::ostream & operator<<(std::ostream &os, const struct sockaddr_in6 &addr);

static inline std::ostream & operator<<(std::ostream &os, const SocketAddress &addr) {
	return os << *addr;
}

static inline std::ostream & operator<<(std::ostream &os, const struct sockaddr_storage &addr) {
	return os << reinterpret_cast<const struct sockaddr &>(addr);
}

std::istream & operator>>(std::istream &is, struct sockaddr_in &addr);
std::istream & operator>>(std::istream &is, struct sockaddr_in6 &addr);
std::istream & operator>>(std::istream &is, SocketAddress &addr);

void convert(SocketAddress &addr, const char str[]);
