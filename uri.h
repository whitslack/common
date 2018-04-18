#include <iosfwd>
#include <string>
#include <utility>

#include "compiler.h"


class URI {

private:
	typedef std::pair<std::string::const_iterator, std::string::const_iterator> span_t;

private:
	std::string uri;
	span_t scheme, ssp;
	span_t authority, userinfo, host, port;
	span_t path, query, fragment;

public:
	explicit URI(const char uri[]) : uri(uri) { this->parse(); }
	explicit URI(const std::string &uri) : uri(uri) { this->parse(); }
	explicit URI(std::string &&uri) : uri(std::move(uri)) { this->parse(); }

	bool _pure is_absolute() const noexcept { return scheme.first != uri.end(); }
	bool _pure is_hierarchical() const noexcept { return authority.first != uri.end() || path.first != uri.end(); }
	bool _pure is_opaque() const noexcept { return this->is_absolute() && !this->is_hierarchical(); }

	std::string _pure get_scheme() const { return { scheme.first, scheme.second }; }
	std::string _pure get_ssp() const { return { ssp.first, ssp.second }; }
	std::string _pure get_authority() const { return { authority.first, authority.second }; }
	std::string _pure get_userinfo() const { return { userinfo.first, userinfo.second }; }
	std::string _pure get_host() const { return { host.first, host.second }; }
	std::string _pure get_port() const { return { port.first, port.second }; }
	std::string _pure get_path() const { return { path.first, path.second }; }
	std::string _pure get_query() const { return { query.first, query.second }; }
	std::string _pure get_fragment() const { return { fragment.first, fragment.second }; }

	const std::string & _pure to_string() const noexcept { return uri; }

private:
	void parse();

};

std::ostream & operator << (std::ostream &os, const URI &uri);
