#pragma once

#include <algorithm>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <string>

#include <sys/types.h>

#include "ci.h"
#include "compiler.h"
#include "nbio.h"

extern const char
		HTTP_REASON_PHRASE_100[], HTTP_REASON_PHRASE_101[],
		HTTP_REASON_PHRASE_200[], HTTP_REASON_PHRASE_201[], HTTP_REASON_PHRASE_202[], HTTP_REASON_PHRASE_203[], HTTP_REASON_PHRASE_204[], HTTP_REASON_PHRASE_205[], HTTP_REASON_PHRASE_206[],
		HTTP_REASON_PHRASE_300[], HTTP_REASON_PHRASE_301[], HTTP_REASON_PHRASE_302[], HTTP_REASON_PHRASE_303[], HTTP_REASON_PHRASE_304[], HTTP_REASON_PHRASE_305[], HTTP_REASON_PHRASE_307[],
		HTTP_REASON_PHRASE_400[], HTTP_REASON_PHRASE_401[], HTTP_REASON_PHRASE_402[], HTTP_REASON_PHRASE_403[], HTTP_REASON_PHRASE_404[], HTTP_REASON_PHRASE_405[], HTTP_REASON_PHRASE_406[], HTTP_REASON_PHRASE_407[], HTTP_REASON_PHRASE_408[], HTTP_REASON_PHRASE_409[], HTTP_REASON_PHRASE_410[], HTTP_REASON_PHRASE_411[], HTTP_REASON_PHRASE_412[], HTTP_REASON_PHRASE_413[], HTTP_REASON_PHRASE_414[], HTTP_REASON_PHRASE_415[], HTTP_REASON_PHRASE_416[], HTTP_REASON_PHRASE_417[], HTTP_REASON_PHRASE_426[], HTTP_REASON_PHRASE_428[],
		HTTP_REASON_PHRASE_429[], HTTP_REASON_PHRASE_431[],
		HTTP_REASON_PHRASE_500[], HTTP_REASON_PHRASE_501[], HTTP_REASON_PHRASE_502[], HTTP_REASON_PHRASE_503[], HTTP_REASON_PHRASE_504[], HTTP_REASON_PHRASE_505[], HTTP_REASON_PHRASE_511[];


template <typename Itr>
static bool any_equal_ci(Itr first, Itr last, const std::string &str) {
	return std::find_if(first, last, [&](const std::pair<std::string, std::string> &elem) { return compare_ci(elem.second, str) == 0; }) != last;
}


class HttpHeaders : public std::multimap<std::string, std::string, less_ci> {

public:
	std::pair<iterator, iterator> split(const std::string &field_name);

};


class HttpRequestHeaders : public HttpHeaders {

public:
	std::string method;
	std::string request_uri;
	std::string protocol_version;

public:
	HttpRequestHeaders() { }
	HttpRequestHeaders(const std::string &method, const std::string &request_uri, const std::string &protocol_version) : method(method), request_uri(request_uri), protocol_version(protocol_version) { }

};

std::istream & operator >> (std::istream &is, HttpRequestHeaders &headers);
std::ostream & operator << (std::ostream &os, const HttpRequestHeaders &headers);


class HttpResponseHeaders : public HttpHeaders {

public:
	std::string protocol_version;
	uint status_code;
	std::string reason_phrase;

public:
	HttpResponseHeaders() { }
	HttpResponseHeaders(const std::string &protocol_version, uint status_code, const std::string &reason_phrase) : protocol_version(protocol_version), status_code(status_code), reason_phrase(reason_phrase) { }

};

std::istream & operator >> (std::istream &is, HttpResponseHeaders &headers);
std::ostream & operator << (std::ostream &os, const HttpResponseHeaders &headers);


std::string rfc2822_date(struct tm &tm, bool is_utc);


class HttpConnectionBase : public Source, public Sink {

private:
	SourceSinkBuf ssb;
	bool read_chunked, write_chunked;
	bool response_headers_read;
	HttpResponseHeaders response_headers;
	ssize_t chunk_rem;
	char buf[3000];

protected:
	HttpConnectionBase(Source *source, Sink *sink);

public:
	void request(const HttpRequestHeaders &request_headers);
	const HttpResponseHeaders & get_response_headers();
	ssize_t read(void *buf, size_t n) override;
	size_t write(const void *buf, size_t n, bool more = false) override;

};


class HttpConnection : public HttpConnectionBase {

private:
	Socket socket;

public:
	HttpConnection(const std::string &host, uint16_t port = 80);

};
