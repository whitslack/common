#pragma once

#include <ctime>
#include <map>
#include <string_view>

#include "ci.h"
#include "io.h"

extern const char
		HTTP_REASON_PHRASE_100[], HTTP_REASON_PHRASE_101[],
		HTTP_REASON_PHRASE_200[], HTTP_REASON_PHRASE_201[], HTTP_REASON_PHRASE_202[], HTTP_REASON_PHRASE_203[], HTTP_REASON_PHRASE_204[], HTTP_REASON_PHRASE_205[], HTTP_REASON_PHRASE_206[],
		HTTP_REASON_PHRASE_300[], HTTP_REASON_PHRASE_301[], HTTP_REASON_PHRASE_302[], HTTP_REASON_PHRASE_303[], HTTP_REASON_PHRASE_304[], HTTP_REASON_PHRASE_305[], HTTP_REASON_PHRASE_307[],
		HTTP_REASON_PHRASE_400[], HTTP_REASON_PHRASE_401[], HTTP_REASON_PHRASE_402[], HTTP_REASON_PHRASE_403[], HTTP_REASON_PHRASE_404[], HTTP_REASON_PHRASE_405[], HTTP_REASON_PHRASE_406[], HTTP_REASON_PHRASE_407[], HTTP_REASON_PHRASE_408[], HTTP_REASON_PHRASE_409[], HTTP_REASON_PHRASE_410[], HTTP_REASON_PHRASE_411[], HTTP_REASON_PHRASE_412[], HTTP_REASON_PHRASE_413[], HTTP_REASON_PHRASE_414[], HTTP_REASON_PHRASE_415[], HTTP_REASON_PHRASE_416[], HTTP_REASON_PHRASE_417[], HTTP_REASON_PHRASE_426[], HTTP_REASON_PHRASE_428[],
		HTTP_REASON_PHRASE_429[], HTTP_REASON_PHRASE_431[],
		HTTP_REASON_PHRASE_500[], HTTP_REASON_PHRASE_501[], HTTP_REASON_PHRASE_502[], HTTP_REASON_PHRASE_503[], HTTP_REASON_PHRASE_504[], HTTP_REASON_PHRASE_505[], HTTP_REASON_PHRASE_511[];


class HttpHeaders : public std::multimap<std::string, std::string, ci::less> {

public:
	const_iterator find_token(std::string_view field_name, std::string_view token) const _pure;

};


class HttpRequestHeaders : public HttpHeaders {

public:
	std::string method;
	std::string request_uri;
	std::string protocol_version;

public:
	HttpRequestHeaders() = default;
	HttpRequestHeaders(std::string method, std::string request_uri, std::string protocol_version) : method(std::move(method)), request_uri(std::move(request_uri)), protocol_version(std::move(protocol_version)) { }

};

std::istream & operator>>(std::istream &is, HttpRequestHeaders &headers);
std::ostream & operator<<(std::ostream &os, const HttpRequestHeaders &headers);


class HttpResponseHeaders : public HttpHeaders {

public:
	std::string protocol_version;
	unsigned status_code;
	std::string reason_phrase;

public:
	HttpResponseHeaders() = default;
	HttpResponseHeaders(std::string protocol_version, unsigned status_code, std::string reason_phrase) : protocol_version(std::move(protocol_version)), status_code(status_code), reason_phrase(std::move(reason_phrase)) { }

};

std::istream & operator>>(std::istream &is, HttpResponseHeaders &headers);
std::ostream & operator<<(std::ostream &os, const HttpResponseHeaders &headers);


std::string rfc2822_date(const struct std::tm &tm);
std::time_t rfc2822_date(std::string_view sv);


class ChunkedSource : public Source {

private:
	Source &source;
	size_t chunk_rem;
	enum { Size, Size_CR, Extensions, Extensions_CR, Data, Data_End, Data_CR, End } state;

public:
	explicit ChunkedSource(Source &source) noexcept : source(source), chunk_rem(), state() { }

public:
	void reset() noexcept { chunk_rem = 0, state = Size; }
	_nodiscard ssize_t read(void *buf, size_t n) override;

};


class ChunkedSink : public Sink {

private:
	Sink &sink;
	size_t write_size;
	enum { Idle, Size, Size_CR, Size_LF, Data, Data_CR, Data_LF, End } state;

public:
	explicit ChunkedSink(Sink &sink) noexcept : sink(sink), write_size(), state() { }

public:
	void reset() noexcept { write_size = 0, state = Idle; }
	_nodiscard size_t write(const void *buf, size_t n) override;
	bool flush() override;

private:
	size_t write(const void *buf, size_t n, bool flush);

};
