#include <iosfwd>
#include <string>
#include <string_view>

#include "compiler.h"


class URI {

private:
	std::string uri;
	std::string_view scheme;
	std::string_view authority, userinfo, host, port;
	std::string_view path, query, fragment;

public:
	explicit URI(const char uri[]) : uri(uri) { this->parse(); }
	explicit URI(const std::string &uri) : uri(uri) { this->parse(); }
	explicit URI(std::string &&uri) : uri(std::move(uri)) { this->parse(); }

	bool _pure is_opaque() const noexcept { return !authority.data(); }

	std::string_view _pure get_scheme() const noexcept { return scheme; }
	std::string_view _pure get_authority() const noexcept { return authority; }
	std::string_view _pure get_userinfo() const noexcept { return userinfo; }
	std::string_view _pure get_host() const noexcept { return host; }
	std::string_view _pure get_port() const noexcept { return port; }
	std::string_view _pure get_path() const noexcept { return path; }
	std::string_view _pure get_query() const noexcept { return query; }
	std::string_view _pure get_fragment() const noexcept { return fragment; }

	const std::string & _pure to_string() const noexcept { return uri; }

private:
	void parse();

};

std::ostream & operator << (std::ostream &os, const URI &uri);
