#include <string>
#include <string_view>

#include "compiler.h"


class URI {

private:
	std::string uri_;
	std::string_view scheme_, authority_, userinfo_, host_, port_, path_, query_, fragment_;

public:
	explicit URI(std::string uri) : uri_(std::move(uri)) { this->parse(); }

	_pure operator const std::string & () const noexcept { return uri_; }

	bool _pure is_opaque() const noexcept { return !authority_.data(); }

	std::string_view _pure scheme() const noexcept { return scheme_; }
	std::string_view _pure authority() const noexcept { return authority_; }
	std::string_view _pure userinfo() const noexcept { return userinfo_; }
	std::string_view _pure host() const noexcept { return host_; }
	std::string_view _pure port() const noexcept { return port_; }
	std::string_view _pure path() const noexcept { return path_; }
	std::string_view _pure query() const noexcept { return query_; }
	std::string_view _pure fragment() const noexcept { return fragment_; }

	const std::string & _const to_string() const noexcept { return uri_; }

private:
	void parse();

};
