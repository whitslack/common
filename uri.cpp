#include "uri.h"

#include <stdexcept>

#include "regex.h"

template <typename T, size_t N>
constexpr size_t countof(T (&)[N]) { return N; }

#define _URI_reference_ "(" _absoluteURI_ "|" _relativeURI_ ")?" "(#" _fragment_ ")?"

#define _absoluteURI_ _scheme_ ":" "(" _hier_part_ "|" _opaque_part_ ")"
#define _relativeURI_ "(" _net_path_ "|" _abs_path_ "|" _rel_path_ ")" "(\\?" _query_ ")?"
#define _fragment_ "(" _uric_ "*" ")"

#define _scheme_ "(" "[A-Za-z][-+.0-9A-Za-z0-9]*" ")"
#define _hier_part_ "(" _net_path_ "|" _abs_path_ ")" "(\\?" _query_ ")?"
#define _opaque_part_ _uric_no_slash_ _uric_ "*"
#define _net_path_ "//" _authority_ "(" _abs_path_ ")?"
#define _abs_path_ "/" _path_segments_
#define _rel_path_ _rel_segment_ "(" _abs_path_ ")?"
#define _query_ "(" _uric_ "*" ")"
#define _uric_ "(" "[][!$&'()*+,./0-9:;=?@A-Z_a-z~-]" "|" _escaped_ ")"

#define _uric_no_slash_ "(" "[-!$&'()*+,.0-9:;=?@A-Z_a-z~]" "|" _escaped_ ")"
#define _authority_ "(" _server_ "|" _reg_name_ ")"
#define _path_segments_ _segment_ "(/" _segment_ ")*"
#define _rel_segment_ "(" "[-!$&'()*+,.0-9;=@A-Z_a-z~]" "|" _escaped_ ")+"
#define _escaped_ "%[[:xdigit:]]{2}"

#define _server_ "(" _userinfo_ "@)?" _hostport_
#define _reg_name_ "(" "[-!$&'()*+,.0-9:;=@A-Z_a-z~]" "|" _escaped_ ")+"
#define _segment_ _pchar_ "*" "(;" _param_ ")*"

#define _userinfo_ "(" "(" "[-!$&'()*+,.0-9:;=A-Z_a-z~]" "|" _escaped_ ")*" ")"
#define _hostport_ _host_ "(:" _port_ ")?"
#define _pchar_ "(" "[-!$&'()*+,.0-9:=@A-Z_a-z~]" "|" _escaped_ ")"
#define _param_ _pchar_ "*"

#define _host_ "(" _hostname_ "|" _IPv4address_ "|" _IPv6reference_ ")"
#define _port_ "(" _digit_ "*" ")"

#define _hostname_ "(" _domainlabel_ "\\.)*" _toplabel_ "\\.?"
#define _IPv4address_ _digit_ "{1,3}" "(\\." _digit_ "{1,3}){3}"
#define _IPv6reference_ "\\[" _IPv6address_ "\\]"
#define _digit_ "[0-9]"

#define _domainlabel_ "[0-9A-Za-z]" "(" "[-0-9A-Za-z]*[0-9A-Za-z]" ")?"
#define _toplabel_ "[A-Za-z]" "(" "[-0-9A-Za-z]*[0-9A-Za-z]" ")?"
#define _IPv6address_ _hexpart_ "(:" _IPv4address_ ")?"

#define _hexpart_ "(" _hexseq_ "|" _hexseq_ "::" "(" _hexseq_ ")?" "|" "::" "(" _hexseq_ ")?" ")"
#define _hexseq_ _hex4_ "(:" _hex4_ ")*"
#define _hex4_ "[[:xdigit:]]{1,4}"

static Regex parse_regex("^" _URI_reference_ "$");


void URI::parse() {
	regmatch_t matches[98];
	if (!parse_regex.exec(uri.c_str(), countof(matches), matches)) {
		throw std::invalid_argument("invalid URI");
	}
	scheme = { uri.begin() + matches[2].rm_so, uri.begin() + matches[2].rm_eo };
	if (scheme.first == scheme.second) {
		ssp = { uri.begin() + matches[1].rm_so, uri.begin() + matches[1].rm_eo };
		authority = { uri.begin() + matches[47].rm_so, uri.begin() + matches[47].rm_eo };
		userinfo = { uri.begin() + matches[49].rm_so, uri.begin() + matches[49].rm_eo };
		host = { uri.begin() + matches[51].rm_so, uri.begin() + matches[51].rm_eo };
		port = { uri.begin() + matches[66].rm_so, uri.begin() + matches[66].rm_eo };
		if (matches[47].rm_so == matches[47].rm_eo) {
			path = { uri.begin() + matches[46].rm_so, uri.begin() + matches[46].rm_eo };
		}
		else {
			path = { uri.begin() + matches[68].rm_so, uri.begin() + matches[68].rm_eo };
		}
		query = { uri.begin() + matches[93].rm_so, uri.begin() + matches[93].rm_eo };
	}
	else {
		ssp = { uri.begin() + matches[3].rm_so, uri.begin() + matches[3].rm_eo };
		authority = { uri.begin() + matches[5].rm_so, uri.begin() + matches[5].rm_eo };
		userinfo = { uri.begin() + matches[7].rm_so, uri.begin() + matches[7].rm_eo };
		host = { uri.begin() + matches[9].rm_so, uri.begin() + matches[9].rm_eo };
		port = { uri.begin() + matches[24].rm_so, uri.begin() + matches[24].rm_eo };
		if (matches[5].rm_so == matches[5].rm_eo) {
			path = { uri.begin() + matches[4].rm_so, uri.begin() + matches[4].rm_eo };
		}
		else {
			path = { uri.begin() + matches[26].rm_so, uri.begin() + matches[26].rm_eo };
		}
		query = { uri.begin() + matches[42].rm_so, uri.begin() + matches[42].rm_eo };
	}
	fragment = { uri.begin() + matches[96].rm_so, uri.begin() + matches[96].rm_eo };
}

std::ostream & operator << (std::ostream &os, const URI &uri) {
	return os << uri.to_string();
}


/*
#include <iostream>

int main() {
	std::ios_base::sync_with_stdio(false);
	std::string line;
	std::cerr << std::boolalpha;
	while (getline(std::cin, line)) {
		regmatch_t matches[98];
		if (parse_regex.exec(line.c_str(), countof(matches), matches)) {
			for (size_t i = 0; i < countof(matches); ++i) {
				(std::cout << " [" << i << "]=\"").write(line.data() + matches[i].rm_so, matches[i].rm_eo - matches[i].rm_so) << '"';
			}
			std::cout << std::endl;
			URI uri(std::move(line));
			std::cout << " scheme = \"" << uri.get_scheme() << '"' << std::endl;
			std::cout << " ssp = \"" << uri.get_ssp() << '"' << std::endl;
			std::cout << " authority = \"" << uri.get_authority() << '"' << std::endl;
			std::cout << " userinfo = \"" << uri.get_userinfo() << '"' << std::endl;
			std::cout << " host = \"" << uri.get_host() << '"' << std::endl;
			std::cout << " port = \"" << uri.get_port() << '"' << std::endl;
			std::cout << " path = \"" << uri.get_path() << '"' << std::endl;
			std::cout << " query = \"" << uri.get_query() << '"' << std::endl;
			std::cout << " fragment = \"" << uri.get_fragment() << '"' << std::endl;
		}
		else {
			std::cerr << false << std::endl;
		}
	}
	return 0;
}
*/
