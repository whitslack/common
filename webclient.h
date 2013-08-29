#include <memory>

#include "http.h"
#include "nbio.h"

#ifdef HTTP_GZIP
#include "gzip.h"
#endif

#ifdef HTTP_TLS
#include "tls.h"
#endif


class HttpConnectionBase : public Source, public Sink {

private:
	Source * const source;
	Sink * const sink;
	LimitedSource limited_source;
	ChunkedSource chunked_source;
	ChunkedSink chunked_sink;
#ifdef HTTP_GZIP
	std::unique_ptr<GZipSource> gzip_source; // [C++14] std::optional
#endif
	Source *read_source;
	Sink *write_sink;
	bool read_chunked, response_headers_read;
	HttpResponseHeaders response_headers;

protected:
	HttpConnectionBase(Source *source, Sink *sink) : source(source), sink(sink), limited_source(source, 0), chunked_source(source), chunked_sink(sink), read_source(), write_sink(), read_chunked(), response_headers_read() { }

public:
	void request(HttpRequestHeaders &request_headers);
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
