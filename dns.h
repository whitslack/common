#include <system_error>

#include <netdb.h>

#include "compiler.h"


class GAICategory : public std::error_category {

public:
	const char * name() const noexcept override _const;
	std::string message(int condition) const noexcept override _const;

};


class GAIResults {
	friend GAIResults getaddrinfo(const char [], const char [], int, int, int, int);

public:
	class Iterator : public std::iterator<std::forward_iterator_tag, addrinfo, void> {
		friend class GAIResults;
	private:
		addrinfo *ptr;
	public:
		Iterator() : ptr() { }
		operator addrinfo * () const { return ptr; }
		addrinfo * operator -> () const { return ptr; }
		Iterator & operator ++ () {
			ptr = ptr->ai_next;
			return *this;
		}
		Iterator operator ++ (int) {
			Iterator copy(*this);
			ptr = ptr->ai_next;
			return copy;
		}
	private:
		explicit Iterator(addrinfo *ptr) : ptr(ptr) { }
	};

private:
	addrinfo *res;

public:
	GAIResults(GAIResults &&move) : res(move.res) { move.res = nullptr; }
	~GAIResults() { ::freeaddrinfo(res); }

	Iterator begin() const { return Iterator(res); }
	Iterator end() const { return Iterator(); }

private:
	explicit GAIResults(addrinfo *res) : res(res) { }
	GAIResults(const GAIResults &) = delete;
	GAIResults & operator = (const GAIResults &) = delete;

};

GAIResults getaddrinfo(const char host[], const char service[] = nullptr, int family = AF_UNSPEC, int type = SOCK_STREAM, int protocol = 0, int flags = AI_V4MAPPED | AI_ADDRCONFIG);


std::ostream & operator << (std::ostream &os, const struct sockaddr &addr);
std::ostream & operator << (std::ostream &os, const struct sockaddr_in &addr);
std::ostream & operator << (std::ostream &os, const struct sockaddr_in6 &addr);

static inline std::ostream & operator << (std::ostream &os, const struct sockaddr_storage &addr) {
	return os << reinterpret_cast<const struct sockaddr &>(addr);
}
