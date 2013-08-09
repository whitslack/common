#include "http.h"

#include <iomanip>
#include <sstream>

template <typename T, size_t N>
constexpr size_t countof(T (&)[N]) { return N; }

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

int compare_ci(const std::string &str1, const std::string &str2) {
	auto it1 = str1.begin(), end1 = str1.end(), it2 = str2.begin(), end2 = str2.end();
	for (;;) {
		if (it1 == end1) {
			return it2 == end2 ? 0 : -1;
		}
		if (it2 == end2) {
			return 1;
		}
		int c1 = std::toupper(*it1++), c2 = std::toupper(*it2++);
		if (c1 != c2) {
			return c1 < c2 ? -1 : 1;
		}
	}
}

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
	for (int c; !std::istream::traits_type::eq_int_type(c = is.peek(), std::istream::traits_type::eof()) && c >= 0 && c < static_cast<int>(countof(map)) && map[c];) {
		token.push_back(static_cast<char>(is.get()));
	}
	return is;
}

static std::istream & read_word(std::istream &is, std::string &word) {
	word.clear();
	for (int c; !std::istream::traits_type::eq_int_type(c = is.peek(), std::istream::traits_type::eof()) && !std::iscntrl(c) && c != ' ';) {
		word.push_back(static_cast<char>(is.get()));
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
			text.push_back(static_cast<char>(is.get()));
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
						// C++11: field_value.pop_back();
						field_value.resize(field_value.size() - 1);
					}
					// C++11: headers.emplace(field_name, field_value);
					headers.insert({ field_name, field_value });
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

std::pair<HttpHeaders::iterator, HttpHeaders::iterator> HttpHeaders::split(const std::string &field_name) {
	auto pair = this->equal_range(field_name);
	for (auto it = pair.first; it != pair.second; ++it) {
		auto &field_values = it->second;
		size_t i = 0, n = field_values.size();
		for (; i < n && std::isblank(field_values[i]); ++i);
		for (size_t j = i; j < n;) {
			char c = field_values[j];
			if (c == ',') {
				size_t k = j;
				for (; k > i && std::isblank(field_values[k - 1]); --k);
				// [C++11] this->emplace_hint(it, it->first, field_values.substr(i, k - i));
				auto inserted = this->insert(it, { it->first, field_values.substr(i, k - i) });
				if (it == pair.first) {
					pair.first = inserted;
				}
				while (++j < n && std::isblank(field_values[j]));
				i = j;
				continue;
			}
			else if (c == '"') {
				while (++j < n && (c = field_values[j]) != '"') {
					if ((c = field_values[j]) == '\\') {
						++j;
					}
				}
			}
			++j;
		}
		size_t k = n;
		for (; k > i && std::isblank(field_values[k - 1]); --k);
		if (i > 0 || k < n) {
			field_values.assign(field_values, i, k - i);
		}
	}
	return pair;
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
	// C++11: headers.reason_phrase.pop_back();
	headers.reason_phrase.resize(headers.reason_phrase.size() - 1);
	return read_header_fields(is, headers);
}

std::ostream & operator << (std::ostream &os, const HttpResponseHeaders &headers) {
	return write_header_fields(os << headers.protocol_version << ' ' << headers.status_code << ' ' << headers.reason_phrase << "\r\n", headers);
}


std::string rfc2822_date(struct tm &tm, bool is_utc) {
	static const char *weekday_name[7] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
	static const char *month_name[12] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
	std::ostringstream oss;
	oss << weekday_name[tm.tm_wday] << ',' << ' ' << tm.tm_mday << ' ' << month_name[tm.tm_mon] << ' ' << 1900 + tm.tm_year << ' ' << std::setfill('0') << std::setw(2) << tm.tm_hour << ':' << std::setw(2) << tm.tm_min << ':' << std::setw(2) << tm.tm_sec << ' ';
	if (is_utc) {
		oss << "UTC";
	}
	else {
		char buf[8];
		std::strftime(buf, sizeof buf, "%z", &tm);
		oss << buf;
	}
	return oss.str();
}


HttpConnectionBase::HttpConnectionBase(Source *source, Sink *sink) : ssb(source, sink), read_chunked(), write_chunked(), response_headers_read(), chunk_rem() {
	ssb.pubsetbuf(buf, sizeof buf);
}

void HttpConnectionBase::request(const HttpRequestHeaders &request_headers) {
	std::ostream os(&ssb);
	os << request_headers << std::flush;
	write_chunked = request_headers.find("Content-Length") == request_headers.end();
	response_headers_read = false;
}

const HttpResponseHeaders & HttpConnectionBase::get_response_headers() {
	if (!response_headers_read) {
		std::istream(&ssb) >> response_headers;
		auto transfer_encoding_range = response_headers.split("Transfer-Encoding");
		read_chunked = any_equal_ci(transfer_encoding_range.first, transfer_encoding_range.second, "chunked");
		if (read_chunked) {
			chunk_rem = 0;
		}
		else {
			auto content_length_itr = response_headers.find("Content-Length");
			chunk_rem = content_length_itr == response_headers.end() ? -1 : std::stoul(content_length_itr->second);
		}
		response_headers_read = true;
	}
	return response_headers;
}

ssize_t HttpConnectionBase::read(void *buf, size_t n) {
	this->get_response_headers();
	if (!read_chunked) {
		if (chunk_rem < 0) {
			return ssb.sgetn(reinterpret_cast<char *>(buf), n);
		}
		if (chunk_rem == 0) {
			return -1;
		}
		std::streamsize r = ssb.sgetn(reinterpret_cast<char *>(buf), std::min(n, static_cast<size_t>(chunk_rem)));
		if (r > 0) {
			chunk_rem -= r;
		}
		return r;
	}
	std::istream is(&ssb);
	if (chunk_rem <= 0) {
		if (chunk_rem < 0) {
			return -1;
		}
		is >> std::hex >> chunk_rem;
		if (!skip_crlf(is)) {
			throw std::ios_base::failure("bad chunk");
		}
		if (chunk_rem == 0) {
			chunk_rem = -1;
			if (!skip_crlf(is)) {
				throw std::ios_base::failure("HTTP trailers not supported");
			}
			return -1;
		}
	}
	is.read(static_cast<char *>(buf), std::min(n, static_cast<size_t>(chunk_rem)));
	n = is.gcount();
	if (n > 0 && (chunk_rem -= n) == 0 && !skip_crlf(is)) {
		throw std::ios_base::failure("bad chunk");
	}
	return n;
}

size_t HttpConnectionBase::write(const void *buf, size_t n, bool more) {
	if (!write_chunked) {
		std::streamsize w = ssb.sputn(reinterpret_cast<const char *>(buf), n);
		if (!more) {
			ssb.pubsync();
		}
		return w;
	}
	std::ostream os(&ssb);
	(os << std::hex << n << "\r\n").write(static_cast<const char *>(buf), n) << "\r\n";
	if (!more) {
		os << 0 << "\r\n" << std::flush;
	}
	return os ? n : 0;
}


HttpConnection::HttpConnection(const std::string &host, uint16_t port) : HttpConnectionBase(&socket, &socket) {
	for (auto &info : getaddrinfo(host.c_str())) {
		if (info.ai_family == AF_INET) {
			reinterpret_cast<sockaddr_in *>(info.ai_addr)->sin_port = htobe16(port);
		}
		else if (info.ai_family == AF_INET6) {
			reinterpret_cast<sockaddr_in6 *>(info.ai_addr)->sin6_port = htobe16(port);
		}
		else {
			continue;
		}
		try {
			socket.open(info.ai_family, info.ai_socktype | SOCK_CLOEXEC, info.ai_protocol);
			int optval = 1;
			socket.setsockopt(SOL_SOCKET, SO_KEEPALIVE, &optval, static_cast<socklen_t>(sizeof optval));
			socket.connect(info.ai_addr, info.ai_addrlen);
			return;
		}
		catch (...) {
			continue;
		}
	}
	throw std::system_error(errno, std::system_category(), "connect");
}
