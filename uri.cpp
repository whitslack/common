#include "uri.h"

#include <regex>
#include <stdexcept>


// https://tools.ietf.org/html/rfc3986#appendix-A

#define _URI_ _scheme_ ":" _hier_part_ "(?:" "\\?" _query_ ")?" "(?:" "#" _fragment_ ")?"

#define _hier_part_ "(?:" "//" _authority_ "(" _path_abempty_ ")" "|" "(" "/" "?" "(?:" _segment_nz_ _path_abempty_ ")?" ")" ")"

#define _scheme_ "(" "[A-Za-z][-A-Za-z0-9+.]*" ")"

#define _authority_ "(" "(?:" _userinfo_ "@" ")?" _host_ "(?:" ":" _port_ ")?" ")"
#define _userinfo_ "(" "(?:" "[" _unreserved_ _sub_delims_ ":" "]" "|" _pct_encoded_ ")*" ")"
#define _host_ "(" _IP_literal_ "|" _IPv4address_ "|" _reg_name_ ")"
#define _port_ "(" "[0-9]*" ")"

#define _IP_literal_ "(?:" "\\[" "(?:" _IPv6address_ "|" _IPvFuture_ ")" "\\]" ")"

#define _IPvFuture_ "(?:" "v" "[[:xdigit:]]+" "\\." "[" _unreserved_ _sub_delims_ ":" "]+" ")"

#define _IPv6address_ \
		"(?:" _h16_ ":" "(?:" _h16_ ":" "(?:" _h16_ ":" "(?:" _h16_ ":" "(?:" _h16_ ":" "(?:" _h16_ ":" "(?:" _h16_ ":" \
		                                 "(?:"           _h16_          "|" ":"                   ")"     \
		"|"                              "(?:"       ":" _h16_          "|"     _IPv4address_     ")" ")" \
		"|"                              "(?:" "(?:" ":" _h16_ "){1,2}" "|" ":" _IPv4address_ "?" ")" ")" \
		"|"     "(?:" ":" _h16_ ")?"     "(?:" "(?:" ":" _h16_ "){1,2}" "|" ":" _IPv4address_ "?" ")" ")" \
		"|"     "(?:" ":" _h16_ "){0,2}" "(?:" "(?:" ":" _h16_ "){1,2}" "|" ":" _IPv4address_ "?" ")" ")" \
		"|"     "(?:" ":" _h16_ "){0,3}" "(?:" "(?:" ":" _h16_ "){1,2}" "|" ":" _IPv4address_ "?" ")" ")" \
		"|"     "(?:" ":" _h16_ "){0,4}" "(?:" "(?:" ":" _h16_ "){1,2}" "|" ":" _IPv4address_ "?" ")" ")" \
		"|" ":" "(?:" ":" _h16_ "){0,5}" "(?:" "(?:" ":" _h16_ "){1,2}" "|" ":" _IPv4address_ "?" ")" ")"

#define _h16_ "[[:xdigit:]]{1,4}"
#define _IPv4address_ "(?:" _dec_octet_ "(?:" "\\." _dec_octet_ "){3}" ")"

#define _dec_octet_ "(?:" "[0-9]" "|" "[1-9][0-9]" "|" "1[0-9]{2}" "|" "2[0-4][0-9]" "|" "25[0-5]" ")"

#define _reg_name_ "(?:" "[" _unreserved_ _sub_delims_ "]" "|" _pct_encoded_ ")*"

#define _path_abempty_ "(?:" "/" _segment_ ")*"

#define _segment_ _pchar_ "*"
#define _segment_nz_ _pchar_ "+"

#define _pchar_ "(?:" "[" _unreserved_ _sub_delims_ ":@" "]" "|" _pct_encoded_ ")"

#define _query_ "(" "(?:" "[" _unreserved_ _sub_delims_ ":@/?" "]" "|" _pct_encoded_ ")*" ")"

#define _fragment_ "(" "(?:" "[" _unreserved_ _sub_delims_ ":@/?" "]" "|" _pct_encoded_ ")*" ")"

#define _pct_encoded_ "%" "[[:xdigit:]]{2}"

#define _unreserved_ "-A-Za-z0-9._~"
#define _sub_delims_ "!$&'()*+,;="

static const std::regex parse_regex(_URI_, std::regex::ECMAScript | std::regex::optimize);


void URI::parse() {
	std::smatch matches;
	if (_unlikely(!std::regex_match(uri_, matches, parse_regex))) {
		throw std::invalid_argument("invalid URI");
	}
	matches[1].matched ? scheme_ = { &*matches[1].first, static_cast<size_t>(matches[1].length()) } : scheme_ = { };
	matches[2].matched ? authority_ = { &*matches[2].first, static_cast<size_t>(matches[2].length()) } : authority_ = { };
	matches[3].matched ? userinfo_ = { &*matches[3].first, static_cast<size_t>(matches[3].length()) } : userinfo_ = { };
	matches[4].matched ? host_ = { &*matches[4].first, static_cast<size_t>(matches[4].length()) } : host_ = { };
	matches[5].matched ? port_ = { &*matches[5].first, static_cast<size_t>(matches[5].length()) } : port_ = { };
	if (matches[6].matched) {
		path_ = { &*matches[6].first, static_cast<size_t>(matches[6].length()) };
	}
	else if (matches[7].matched) {
		path_ = { &*matches[7].first, static_cast<size_t>(matches[7].length()) };
	}
	else {
		path_ = { };
	}
	matches[8].matched ? query_ = { &*matches[8].first, static_cast<size_t>(matches[8].length()) } : query_ = { };
	matches[9].matched ? fragment_ = { &*matches[9].first, static_cast<size_t>(matches[9].length()) } : fragment_ = { };
}


/*
#include <iostream>

int main() {
	std::ios_base::sync_with_stdio(false);
	std::cout << std::boolalpha;
	std::clog << std::boolalpha;
	for (std::string line; getline(std::cin, line);) {
		std::smatch matches;
		if (std::regex_match(line, matches, parse_regex)) {
			for (size_t i = 0; i < std::size(matches); ++i) {
				(std::clog << " [" << i << "]=\"").write(&*matches[i].first, matches[i].length()) << '"';
			}
			std::clog << std::endl;
			URI uri(std::move(line));
			std::cout << " scheme = \"" << uri.scheme() << "\"\n"
					" authority = \"" << uri.authority() << "\"\n"
					" userinfo = \"" << uri.userinfo() << "\"\n"
					" host = \"" << uri.host() << "\"\n"
					" port = \"" << uri.port() << "\"\n"
					" path = \"" << uri.path() << "\"\n"
					" query = \"" << uri.query() << "\"\n"
					" fragment = \"" << uri.fragment() << "\"\n"
					" opaque = " << uri.is_opaque() << std::endl;
		}
		else {
			std::clog << false << std::endl;
		}
	}
	return 0;
}
*/
