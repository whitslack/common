#include "http.h"

#include <iomanip>
#include <regex>
#include <sstream>

#include "memory.h"

#define BITS(l, o) (((size_t(1) << (l)) - 1) << (o))

const char
		HTTP_REASON_PHRASE_100[] = "Continue",
		HTTP_REASON_PHRASE_101[] = "Switching Protocols",
		HTTP_REASON_PHRASE_200[] = "OK",
		HTTP_REASON_PHRASE_201[] = "Created",
		HTTP_REASON_PHRASE_202[] = "Accepted",
		HTTP_REASON_PHRASE_203[] = "Non-Authoritative Information",
		HTTP_REASON_PHRASE_204[] = "No Content",
		HTTP_REASON_PHRASE_205[] = "Reset Content",
		HTTP_REASON_PHRASE_206[] = "Partial Content",
		HTTP_REASON_PHRASE_300[] = "Multiple Choices",
		HTTP_REASON_PHRASE_301[] = "Moved Permanently",
		HTTP_REASON_PHRASE_302[] = "Found",
		HTTP_REASON_PHRASE_303[] = "See Other",
		HTTP_REASON_PHRASE_304[] = "Not Modified",
		HTTP_REASON_PHRASE_305[] = "Use Proxy",
		HTTP_REASON_PHRASE_307[] = "Temporary Redirect",
		HTTP_REASON_PHRASE_400[] = "Bad Request",
		HTTP_REASON_PHRASE_401[] = "Unauthorized",
		HTTP_REASON_PHRASE_402[] = "Payment Required",
		HTTP_REASON_PHRASE_403[] = "Forbidden",
		HTTP_REASON_PHRASE_404[] = "Not Found",
		HTTP_REASON_PHRASE_405[] = "Method Not Allowed",
		HTTP_REASON_PHRASE_406[] = "Not Acceptable",
		HTTP_REASON_PHRASE_407[] = "Proxy Authentication Required",
		HTTP_REASON_PHRASE_408[] = "Request Time-out",
		HTTP_REASON_PHRASE_409[] = "Conflict",
		HTTP_REASON_PHRASE_410[] = "Gone",
		HTTP_REASON_PHRASE_411[] = "Length Required",
		HTTP_REASON_PHRASE_412[] = "Precondition Failed",
		HTTP_REASON_PHRASE_413[] = "Request Entity Too Large",
		HTTP_REASON_PHRASE_414[] = "Request-URI Too Large",
		HTTP_REASON_PHRASE_415[] = "Unsupported Media Type",
		HTTP_REASON_PHRASE_416[] = "Requested Range Not Satisfiable",
		HTTP_REASON_PHRASE_417[] = "Expectation Failed",
		HTTP_REASON_PHRASE_426[] = "Upgrade Required",
		HTTP_REASON_PHRASE_428[] = "Precondition Required",
		HTTP_REASON_PHRASE_429[] = "Too Many Requests",
		HTTP_REASON_PHRASE_431[] = "Request Header Fields Too Large",
		HTTP_REASON_PHRASE_500[] = "Internal Server Error",
		HTTP_REASON_PHRASE_501[] = "Not Implemented",
		HTTP_REASON_PHRASE_502[] = "Bad Gateway",
		HTTP_REASON_PHRASE_503[] = "Service Unavailable",
		HTTP_REASON_PHRASE_504[] = "Gateway Time-out",
		HTTP_REASON_PHRASE_505[] = "HTTP Version not supported",
		HTTP_REASON_PHRASE_511[] = "Network Authentication Required";

static bool skip_crlf(std::istream &is) {
	if (is.peek() == '\r') {
		is.get();
		if (is.peek() == '\n') {
			is.get();
			return true;
		}
		is.unget();
	}
	return false;
}

static bool skip_lws(std::istream &is) {
	if (skip_crlf(is)) {
		if (std::isblank(is.peek())) {
			do {
				is.get();
			} while (std::isblank(is.peek()));
			return true;
		}
		is.unget(), is.unget();
	}
	else if (std::isblank(is.peek())) {
		do {
			is.get();
		} while (std::isblank(is.peek()));
		return true;
	}
	return false;
}

static std::istream & read_token(std::istream &is, std::string &token) {
	static const bool map[] = {
		/*NUL*/false, /*SOH*/false, /*STX*/false, /*ETX*/false, /*EOT*/false, /*ENQ*/false, /*ACK*/false, /*BEL*/false,
		/*BS*/ false, /*HT*/ false, /*LF*/ false, /*VT*/ false, /*FF*/ false, /*CR*/ false, /*SO*/ false, /*SI*/ false,
		/*DLE*/false, /*DC1*/false, /*DC2*/false, /*DC3*/false, /*DC4*/false, /*NAK*/false, /*SYN*/false, /*ETB*/false,
		/*CAN*/false, /*EM*/ false, /*SUB*/false, /*ESC*/false, /*FS*/ false, /*GS*/ false, /*RS*/ false, /*US*/ false,
		/*SP*/ false, /*"!"*/ true, /*<">*/false, /*"#"*/ true, /*"$"*/ true, /*"%"*/ true, /*"&"*/ true, /*"'"*/ true,
		/*"("*/false, /*")"*/false, /*"*"*/ true, /*"+"*/ true, /*","*/false, /*"-"*/ true, /*"."*/ true, /*"/"*/false,
		/*"0"*/ true, /*"1"*/ true, /*"2"*/ true, /*"3"*/ true, /*"4"*/ true, /*"5"*/ true, /*"6"*/ true, /*"7"*/ true,
		/*"8"*/ true, /*"9"*/ true, /*":"*/false, /*";"*/false, /*"<"*/false, /*"="*/false, /*">"*/false, /*"?"*/false,
		/*"@"*/false, /*"A"*/ true, /*"B"*/ true, /*"C"*/ true, /*"D"*/ true, /*"E"*/ true, /*"F"*/ true, /*"G"*/ true,
		/*"H"*/ true, /*"I"*/ true, /*"J"*/ true, /*"K"*/ true, /*"L"*/ true, /*"M"*/ true, /*"N"*/ true, /*"O"*/ true,
		/*"P"*/ true, /*"Q"*/ true, /*"R"*/ true, /*"S"*/ true, /*"T"*/ true, /*"U"*/ true, /*"V"*/ true, /*"W"*/ true,
		/*"X"*/ true, /*"Y"*/ true, /*"Z"*/ true, /*"["*/false, /*"\"*/false, /*"]"*/false, /*"^"*/ true, /*"_"*/ true,
		/*"`"*/ true, /*"a"*/ true, /*"b"*/ true, /*"c"*/ true, /*"d"*/ true, /*"e"*/ true, /*"f"*/ true, /*"g"*/ true,
		/*"h"*/ true, /*"i"*/ true, /*"j"*/ true, /*"k"*/ true, /*"l"*/ true, /*"m"*/ true, /*"n"*/ true, /*"o"*/ true,
		/*"p"*/ true, /*"q"*/ true, /*"r"*/ true, /*"s"*/ true, /*"t"*/ true, /*"u"*/ true, /*"v"*/ true, /*"w"*/ true,
		/*"x"*/ true, /*"y"*/ true, /*"z"*/ true, /*"{"*/false, /*"|"*/ true, /*"}"*/false, /*"~"*/ true, /*DEL*/false
	};
	token.clear();
	for (int c; !std::istream::traits_type::eq_int_type(c = is.peek(), std::istream::traits_type::eof()) && c >= 0 && static_cast<size_t>(c) < std::size(map) && map[c];) {
		token.push_back(std::istream::traits_type::to_char_type(is.get()));
	}
	return is;
}

static std::istream & read_word(std::istream &is, std::string &word) {
	word.clear();
	for (int c; !std::istream::traits_type::eq_int_type(c = is.peek(), std::istream::traits_type::eof()) && !std::iscntrl(c) && c != ' ';) {
		word.push_back(std::istream::traits_type::to_char_type(is.get()));
	}
	return is;
}

static std::istream & read_text(std::istream &is, std::string &text) {
	text.clear();
	for (int c; !std::istream::traits_type::eq_int_type(c = is.peek(), std::istream::traits_type::eof());) {
		if (skip_lws(is)) {
			text.push_back(' ');
		}
		else if (std::iscntrl(c)) {
			break;
		}
		else {
			text.push_back(std::istream::traits_type::to_char_type(is.get()));
		}
	}
	return is;
}

static std::istream & read_header_fields(std::istream &is, HttpHeaders &headers) {
	headers.clear();
	for (;;) {
		std::string field_name;
		if (read_token(is, field_name)) {
			if (field_name.empty()) {
				if (skip_crlf(is)) {
					return is;
				}
			}
			else if (is.peek() == ':') {
				is.get();
				skip_lws(is);
				std::string field_value;
				if (read_text(is, field_value)) {
					if (!field_value.empty() && field_value.back() == ' ') {
						field_value.pop_back();
					}
					headers.emplace(field_name, field_value);
					if (skip_crlf(is)) {
						continue;
					}
				}
			}
		}
		throw std::ios_base::failure("bad header field");
	}
}

static std::ostream & write_header_fields(std::ostream &os, const HttpHeaders &headers) {
	for (auto &pair : headers) {
		os << pair.first << ": " << pair.second << "\r\n";
	}
	return os << "\r\n";
}


HttpHeaders::const_iterator HttpHeaders::find_token(const std::string &field_name, const std::string &token) const {
	auto range = this->equal_range(field_name);
	for (auto field_itr = range.first; field_itr != range.second; ++field_itr) {
		auto value_itr = field_itr->second.begin(), value_end = field_itr->second.end();
		while (value_itr < value_end) {
			while ((std::isblank(*value_itr) || *value_itr == ',') && ++value_itr < value_end);
			if (value_itr < value_end && *value_itr == '"') {
				for (auto token_itr = token.begin(); ++value_itr < value_end; ++token_itr) {
					if (*value_itr == '"') {
						if (token_itr == token.end()) {
							do {
								if (++value_itr == value_end || *value_itr == ',') {
									return field_itr;
								}
							} while (std::isblank(*value_itr));
						}
						break;
					}
					if (*value_itr == '\\' && ++value_itr == value_end) {
						break;
					}
					if (token_itr == token.end() || std::toupper(*value_itr) != std::toupper(*token_itr)) {
						while (++value_itr < value_end && *value_itr != '"') {
							if (*value_itr == '\\' && ++value_itr == value_end) {
								break;
							}
						}
						break;
					}
				}
			}
			else {
				for (auto token_itr = token.begin();; ++value_itr, ++token_itr) {
					if (value_itr == value_end || *value_itr == ',') {
						if (token_itr == token.end()) {
							return field_itr;
						}
						break;
					}
					if (token_itr == token.end() || std::toupper(*value_itr) != std::toupper(*token_itr)) {
						while (std::isblank(*value_itr)) {
							if (++value_itr == value_end) {
								return field_itr;
							}
						}
						if (*value_itr == ',') {
							return field_itr;
						}
						break;
					}
				}
			}
			while (value_itr < value_end && *value_itr++ != ',');
		}
	}
	return this->end();
}

std::istream & operator >> (std::istream &is, HttpRequestHeaders &headers) {
	/*
	 * RFC 2616 §4.1:
	 * In the interest of robustness, servers SHOULD ignore any empty
	 * line(s) received where a Request-Line is expected. In other words, if
	 * the server is reading the protocol stream at the beginning of a
	 * message and receives a CRLF first, it should ignore the CRLF.
	 */
	while (skip_crlf(is));
	if (!read_token(is, headers.method) || headers.method.empty() || is.get() != ' ' || !read_word(is, headers.request_uri) || headers.request_uri.empty() || is.get() != ' ' || !read_word(is, headers.protocol_version) || headers.protocol_version.empty() || is.get() != '\r' || is.get() != '\n') {
		throw std::ios_base::failure("bad request line");
	}
	return read_header_fields(is, headers);
}

std::ostream & operator << (std::ostream &os, const HttpRequestHeaders &headers) {
	return write_header_fields(os << headers.method << ' ' << headers.request_uri << ' ' << headers.protocol_version << "\r\n", headers);
}

std::istream & operator >> (std::istream &is, HttpResponseHeaders &headers) {
	if (!read_word(is, headers.protocol_version) || headers.protocol_version.empty() || is.get() != ' ' || !(is >> headers.status_code) || is.get() != ' ' || !std::getline(is, headers.reason_phrase) || headers.reason_phrase.empty() || headers.reason_phrase.back() != '\r') {
		throw std::ios_base::failure("bad status line");
	}
	headers.reason_phrase.pop_back();
	return read_header_fields(is, headers);
}

std::ostream & operator << (std::ostream &os, const HttpResponseHeaders &headers) {
	return write_header_fields(os << headers.protocol_version << ' ' << headers.status_code << ' ' << headers.reason_phrase << "\r\n", headers);
}


std::string rfc2822_date(const struct std::tm &tm) {
	static const char *weekday_name[7] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
	static const char *month_name[12] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
	std::ostringstream oss;
	oss << weekday_name[tm.tm_wday] << ',' << ' ' << tm.tm_mday << ' ' << month_name[tm.tm_mon] << ' ' << 1900 + tm.tm_year << ' ' << std::setfill('0') << std::setw(2) << tm.tm_hour << ':' << std::setw(2) << tm.tm_min << ':' << std::setw(2) << tm.tm_sec << ' ' << std::showpos << std::internal << std::setw(3) << tm.tm_gmtoff / 3600 << std::noshowpos << std::setw(2) << std::abs(tm.tm_gmtoff) / 60 % 60;
	return oss.str();
}

static int _pure digits_to_int(const char * begin, const char * end) noexcept {
	int ret = 0;
	while (begin < end) {
		ret = ret * 10 + (*begin++ - '0');
	}
	return ret;
}

static int _pure str_to_wday(const char str[]) noexcept {
	switch (str[0]) {
		case 'F':
			if (str[1] == 'r' && str[2] == 'i') { // Fri
				return 5;
			}
			break;
		case 'M':
			if (str[1] == 'o' && str[2] == 'n') { // Mon
				return 1;
			}
			break;
		case 'T':
			switch (str[1]) {
				case 'h': // Th
					if (str[2] == 'u') { // Thu
						return 4;
					}
					break;
				case 'u': // Tu
					if (str[2] == 'e') { // Tue
						return 2;
					}
					break;
			}
			break;
		case 'W':
			if (str[1] == 'e' && str[2] == 'd') { // Wed
				return 3;
			}
			break;
		case 'S':
			switch (str[1]) {
				case 'a': // Sa
					if (str[2] == 't') { // Sat
						return 6;
					}
					break;
				case 'u': // Su
					if (str[2] == 'n') { // Sun
						return 0;
					}
					break;
			}
			break;
	}
	return -1;
}

static int _pure str_to_month(const char str[]) noexcept {
	switch (str[0]) {
		case 'A':
			switch (str[1]) {
				case 'p': // Ap
					if (str[2] == 'r') { // Apr
						return 3;
					}
					break;
				case 'u': // Au
					if (str[2] == 'g') { // Aug
						return 7;
					}
					break;
			}
			break;
		case 'D':
			if (str[1] == 'e' && str[2] == 'c') { // Dec
				return 11;
			}
			break;
		case 'F':
			if (str[1] == 'e' && str[2] == 'b') { // Feb
				return 1;
			}
			break;
		case 'J':
			switch (str[1]) {
				case 'a': // Ja
					if (str[2] == 'n') { // Jan
						return 0;
					}
					break;
				case 'u': // Ju
					switch (str[2]) {
						case 'l': // Jul
							return 6;
						case 'n': // Jun
							return 5;
					}
					break;
			}
			break;
		case 'M':
			if (str[1] == 'a') { // Ma
				switch (str[2]) {
					case 'r': // Mar
						return 2;
					case 'y': // May
						return 4;
				}
			}
			break;
		case 'N':
			if (str[1] == 'o' && str[2] == 'v') { // Nov
				return 10;
			}
			break;
		case 'O':
			if (str[1] == 'c' && str[2] == 't') { // Oct
				return 9;
			}
			break;
		case 'S':
			if (str[1] == 'e' && str[2] == 'p') { // Sep
				return 8;
			}
			break;
	}
	return -1;
}

static int _pure str_to_offset(const char str[]) noexcept {
	switch (str[0]) {
		case '+':
			return digits_to_int(str + 1, str + 3) * 3600 + digits_to_int(str + 3, str + 5) * 60;
		case '-':
			return digits_to_int(str + 1, str + 3) * -3600 + digits_to_int(str + 3, str + 5) * -60;
		case 'A':
			if (str[1] == '\0') { // A (Alpha)
				return 1 * 3600;
			}
			break;
		case 'B':
			if (str[1] == '\0') { // B (Bravo)
				return 2 * 3600;
			}
			break;
		case 'C':
			switch (str[1]) {
				case '\0': // C (Charlie)
					return 3 * 3600;
				case 'D': // CD
					if (str[2] == 'T') { // CDT
						return -5 * 3600;
					}
					break;
				case 'S': // CS
					if (str[2] == 'T') { // CST
						return -6 * 3600;
					}
					break;
			}
			break;
		case 'D':
			if (str[1] == '\0') { // D (Delta)
				return 4 * 3600;
			}
			break;
		case 'E':
			switch (str[1]) {
				case '\0': // E (Echo)
					return 5 * 3600;
				case 'D': // ED
					if (str[2] == 'T') { // EDT
						return -4 * 3600;
					}
					break;
				case 'S': // ES
					if (str[2] == 'T') { // EST
						return -5 * 3600;
					}
					break;
			}
			break;
		case 'F':
			if (str[1] == '\0') { // F (Foxtrot)
				return 6 * 3600;
			}
			break;
		case 'G':
			if (str[1] == '\0') { // G (Golf)
				return 7 * 3600;
			}
			break;
		case 'H':
			if (str[1] == '\0') { // H (Hotel)
				return 8 * 3600;
			}
			break;
		case 'I':
			if (str[1] == '\0') { // I (India)
				return 9 * 3600;
			}
			break;
		case 'K':
			if (str[1] == '\0') { // K (Kilo)
				return 10 * 3600;
			}
			break;
		case 'L':
			if (str[1] == '\0') { // L (Lima)
				return 11 * 3600;
			}
			break;
		case 'M':
			switch (str[1]) {
				case '\0': // M (Mike)
					return 12 * 3600;
				case 'D': // MD
					if (str[2] == 'T') { // MDT
						return -6 * 3600;
					}
					break;
				case 'S': // MS
					if (str[2] == 'T') { // MST
						return -7 * 3600;
					}
					break;
			}
			break;
		case 'N':
			if (str[1] == '\0') { // N (November)
				return -1 * 3600;
			}
			break;
		case 'O':
			if (str[1] == '\0') { // O (Oscar)
				return -2 * 3600;
			}
			break;
		case 'P':
			switch (str[1]) {
				case '\0': // P (Papa)
					return -3 * 3600;
				case 'D': // PD
					if (str[2] == 'T') { // PDT
						return -7 * 3600;
					}
					break;
				case 'S': // PS
					if (str[2] == 'T') { // PST
						return -8 * 3600;
					}
					break;
			}
			break;
		case 'Q':
			if (str[1] == '\0') { // Q (Quebec)
				return -4 * 3600;
			}
			break;
		case 'R':
			if (str[1] == '\0') { // R (Romeo)
				return -5 * 3600;
			}
			break;
		case 'S':
			if (str[1] == '\0') { // S (Sierra)
				return -6 * 3600;
			}
			break;
		case 'T':
			if (str[1] == '\0') { // T (Tango)
				return -7 * 3600;
			}
			break;
		case 'U':
			switch (str[1]) {
				case '\0': // U (Uniform)
					return -8 * 3600;
				case 'T': // UT
					switch (str[2]) {
						case '\0': // UT
						case 'C': // UTC
							return 0;
					}
					break;
			}
			break;
		case 'V':
			if (str[1] == '\0') { // V (Victor)
				return -9 * 3600;
			}
			break;
		case 'W':
			if (str[1] == '\0') { // W (Whiskey)
				return -10 * 3600;
			}
			break;
		case 'X':
			if (str[1] == '\0') { // X (X-ray)
				return -11 * 3600;
			}
			break;
		case 'Y':
			if (str[1] == '\0') { // Y (Yankee)
				return -12 * 3600;
			}
			break;
		case 'Z':
			if (str[1] == '\0') { // Z (Zulu)
				return 0;
			}
			break;
	}
	return INT_MIN;
}

std::time_t rfc2822_date(std::string_view sv) {
#define _FWS_ "(?:(?:[ \t]*\r\n)?[ \t]+)"
#define _date_time_ "(?:" _day_of_week_ ",)?" _FWS_ "?" _date_ _FWS_ _time_
#define _day_of_week_ "(Mon|Tue|Wed|Thu|Fri|Sat|Sun)"
#define _date_ "(?:" _FWS_ "?" _day_ _FWS_ _month_ _FWS_ _year_ ")"
#define _year_ "(" "[0-9]{2,}" ")"
#define _month_ "(Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Oct|Nov|Dec)"
#define _day_ "(" "0?[1-9]|[12][1-9]|3[01]" ")"
#define _time_ "(?:" _time_of_day_ _FWS_ _zone_ ")"
#define _time_of_day_ "(?:" _hour_ ":" _minute_ "(?:" ":" _second_ ")?" ")"
#define _hour_ "(" "[01][0-9]|2[0-3]" ")"
#define _minute_ "(" "[0-5][0-9]" ")"
#define _second_ "(" "[0-5][0-9]|60" ")"
#define _zone_ "(" "[-+][0-9]{4}" "|UTC?|GMT|[ECMP][SD]T|[A-IK-Za-ik-z])"
	static const std::regex regex(_date_time_, std::regex::ECMAScript | std::regex::optimize);
	std::cmatch matches;
	struct std::tm tm { };
	int offset;
	if (!std::regex_match(sv.begin(), sv.end(), matches, regex) ||
			matches[1].matched && (tm.tm_wday = str_to_wday(matches[1].first)) < 0 ||
			(tm.tm_mon = str_to_month(matches[3].first)) < 0 ||
			(offset = str_to_offset(matches[8].first)) == INT_MIN) {
		throw std::ios_base::failure("invalid RFC2822 date");
	}
	tm.tm_mday = digits_to_int(matches[2].first, matches[2].second);
	tm.tm_year = digits_to_int(matches[4].first, matches[4].second);
	if (tm.tm_year >= 1000) {
		tm.tm_year -= 1900;
	}
	else if (tm.tm_year < 50) {
		tm.tm_year += 100;
	}
	tm.tm_hour = digits_to_int(matches[5].first, matches[5].second);
	tm.tm_min = digits_to_int(matches[6].first, matches[6].second);
	tm.tm_sec = digits_to_int(matches[7].first, matches[7].second);
	tm.tm_isdst = -1;
	return ::timegm(&tm) - offset;
}


ssize_t ChunkedSource::read(void *buf, size_t n) {
	static const int8_t UNHEX['f' - '0' + 1] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, -1, -1, -1, -1, -1, -1, -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 10, 11, 12, 13, 14, 15 };
	ssize_t r;
	char c;
	for (;;) {
		switch (state) {
			case Size:
				for (;;) {
					if ((r = source.read(&c, 1)) <= 0) {
						goto Exit;
					}
					if (c == ';') {
						state = Extensions;
						break;
					}
					if (c == '\r') {
						state = Size_CR;
						break;
					}
					int8_t v;
					if (c < '0' || c > 'f' || (v = UNHEX[static_cast<uint8_t>(c - '0')]) < 0) {
						throw std::ios_base::failure("invalid chunk size");
					}
					chunk_rem = chunk_rem << 4 | v;
				}
				break;
			case Size_CR:
				if ((r = source.read(&c, 1)) <= 0) {
					goto Exit;
				}
				if (c != '\n') {
					throw std::ios_base::failure("invalid chunk size");
				}
				state = chunk_rem ? Data : End;
				break;
			case Extensions:
				for (;;) {
					if ((r = source.read(&c, 1)) <= 0) {
						goto Exit;
					}
					if (c == '\r') {
						state = Extensions_CR;
						break;
					}
				}
				break;
			case Extensions_CR:
				if ((r = source.read(&c, 1)) <= 0) {
					goto Exit;
				}
				if (c != '\n') {
					if (c != '\r') {
						state = Extensions;
					}
					break;
				}
				state = chunk_rem ? Data : End;
				break;
			case Data:
				if ((r = source.read(buf, std::min(n, chunk_rem))) <= 0) {
					goto Exit;
				}
				if ((chunk_rem -= r) == 0) {
					state = Data_End;
				}
				return r;
			case Data_End:
				if ((r = source.read(&c, 1)) <= 0) {
					goto Exit;
				}
				if (c != '\r') {
					throw std::ios_base::failure("invalid chunk");
				}
				state = Data_CR;
				// fall through
			case Data_CR:
				if ((r = source.read(&c, 1)) <= 0) {
					goto Exit;
				}
				if (c != '\n') {
					throw std::ios_base::failure("invalid chunk");
				}
				state = Size;
				break;
			case End:
				return -1;
		}
	}
Exit:
	if (r < 0) {
		throw std::ios_base::failure("premature End");
	}
	return 0;
}

size_t ChunkedSource::avail() {
	return std::min(source.avail(), chunk_rem);
}


size_t ChunkedSink::write(const void *buf, size_t n) {
	return this->write(buf, n, false);
}

size_t ChunkedSink::write(const void *buf, size_t n, bool flush) {
	static const char HEX[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
	size_t ret = 0, mask;
	char c;
	for (;;) {
		switch (state) {
			case Idle:
				if (n == 0 && !flush) {
					return ret;
				}
				write_size = n;
				state = Size;
				// fall through
			case Size:
				for (;;) {
#if SIZE_MAX >= UINT64_MAX
					if (write_size & BITS(32, 32)) {
						if (write_size & BITS(16, 48)) {
							if (write_size & BITS(8, 56)) {
								if (mask = write_size & BITS(4, 60)) {
									c = HEX[mask >> 60];
								}
								else {
									c = HEX[(mask = write_size & BITS(4, 56)) >> 56];
								}
							}
							else if (mask = write_size & BITS(4, 52)) {
								c = HEX[mask >> 52];
							}
							else {
								c = HEX[(mask = write_size & BITS(4, 48)) >> 48];
							}
						}
						else if (write_size & BITS(8, 40)) {
							if (mask = write_size & BITS(4, 44)) {
								c = HEX[mask >> 44];
							}
							else {
								c = HEX[(mask = write_size & BITS(4, 40)) >> 40];
							}
						}
						else if (mask = write_size & BITS(4, 36)) {
							c = HEX[mask >> 36];
						}
						else {
							c = HEX[(mask = write_size & BITS(4, 32)) >> 32];
						}
					}
					else
#endif
					if (write_size & BITS(16, 16)) {
						if (write_size & BITS(8, 24)) {
							if (mask = write_size & BITS(4, 28)) {
								c = HEX[mask >> 28];
							}
							else {
								c = HEX[(mask = write_size & BITS(4, 24)) >> 24];
							}
						}
						else if (mask = write_size & BITS(4, 20)) {
							c = HEX[mask >> 20];
						}
						else {
							c = HEX[(mask = write_size & BITS(4, 16)) >> 16];
						}
					}
					else if (write_size & BITS(8, 8)) {
						if (mask = write_size & BITS(4, 12)) {
							c = HEX[mask >> 12];
						}
						else {
							c = HEX[(mask = write_size & BITS(4, 8)) >> 8];
						}
					}
					else if (mask = write_size & BITS(4, 4)) {
						c = HEX[mask >> 4];
					}
					else {
						c = HEX[mask = write_size & BITS(4, 0)];
					}
					if (sink.write(&c, 1) == 0) {
						return ret;
					}
					if ((write_size ^= mask) == 0) {
						state = Size_CR;
						break;
					}
				}
				// fall through
			case Size_CR:
				c = '\r';
				if (sink.write(&c, 1) == 0) {
					return ret;
				}
				state = Size_LF;
				// fall through
			case Size_LF:
				c = '\n';
				if (sink.write(&c, 1) == 0) {
					return ret;
				}
				state = n ? Data : End;
				break;
			case Data:
				if ((ret = sink.write(buf, n)) < n) {
					return ret;
				}
				buf = nullptr;
				n = 0;
				state = Data_CR;
				// fall through
			case Data_CR:
				c = '\r';
				if (sink.write(&c, 1) == 0) {
					return ret;
				}
				state = Data_LF;
				// fall through
			case Data_LF:
				c = '\n';
				if (sink.write(&c, 1) == 0) {
					return ret;
				}
				state = Idle;
				break;
			case End:
				if (n > 0) {
					throw std::logic_error("final chunk already sent");
				}
				return ret;
		}
	}
}

bool ChunkedSink::flush() {
	if (state != End) {
		this->write(nullptr, 0, true);
		if (state != End) {
			return false;
		}
	}
	return sink.flush();
}
