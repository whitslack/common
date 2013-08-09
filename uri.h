#include <string>
#include <utility>

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
	URI(const URI &) = default;
	URI & operator = (const URI &) = default;
	URI(URI &&) = default;
	URI & operator = (URI &&) = default;

	bool is_absolute() const { return scheme.first != uri.end(); }
	bool is_hierarchical() const { return authority.first != uri.end() || path.first != uri.end(); }
	bool is_opaque() const { return this->is_absolute() && !this->is_hierarchical(); }

	std::string get_scheme() const { return { scheme.first, scheme.second }; }
	std::string get_ssp() const { return { ssp.first, ssp.second }; }
	std::string get_authority() const { return { authority.first, authority.second }; }
	std::string get_userinfo() const { return { userinfo.first, userinfo.second }; }
	std::string get_host() const { return { host.first, host.second }; }
	std::string get_port() const { return { port.first, port.second }; }
	std::string get_path() const { return { path.first, path.second }; }
	std::string get_query() const { return { query.first, query.second }; }
	std::string get_fragment() const { return { fragment.first, fragment.second }; }

	std::string to_string() const { return uri; }

private:
	void parse();

};

std::ostream & operator << (std::ostream &os, const URI &uri);
