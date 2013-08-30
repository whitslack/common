#include <memory>

#include "http.h"
#include "nbio.h"

#ifdef HTTP_TLS
#include "tls.h"
#endif


class HttpConnectionBase : public Source, public Sink {

private:
	Source * const source;
	Sink * const sink;
	ChunkedSource chunked_source;
	ChunkedSink chunked_sink;
	bool read_chunked, write_chunked;
	bool response_headers_read;
	HttpResponseHeaders response_headers;
	size_t remaining;

protected:
	HttpConnectionBase(Source *source, Sink *sink) : source(source), sink(sink), chunked_source(source), chunked_sink(sink), read_chunked(), write_chunked(), response_headers_read(), remaining() { }

public:
	void request(const HttpRequestHeaders &request_headers);
	const HttpResponseHeaders & get_response_headers();
	ssize_t read(void *buf, size_t n) override;
	size_t write(const void *buf, size_t n, bool more = false) override;
	bool finish() override;

};


class HttpConnection : public HttpConnectionBase {

private:
	Socket socket;
	BufferedSource<1500> buffered_source;
	BufferedSink<1500> buffered_sink;

public:
	HttpConnection(const std::string &host, uint16_t port = 80);

};


#ifdef HTTP_TLS

class HttpsConnection : public HttpConnectionBase {

private:
	TLSSocket tls;

public:
	HttpsConnection(const std::string &host, uint16_t port = 443, const char ca_file[] = nullptr);

};

#endif // defined(HTTP_TLS)
