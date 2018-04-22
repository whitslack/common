#include <optional>

#include "http.h"
#include "socket.h"

#ifdef HTTP_GZIP
#include "gzip.h"
#endif

#ifdef HTTP_TLS
#include "tls.h"
#endif


class HttpConnectionBase : public Source, public Sink {

private:
	Source &source;
	Sink &sink;
	LimitedSource limited_source;
	ChunkedSource chunked_source;
	ChunkedSink chunked_sink;
#ifdef HTTP_GZIP
	std::optional<GZipSource> gzip_source;
#endif
	Source *read_source;
	Sink *write_sink;
	bool read_chunked, response_headers_read;
	HttpResponseHeaders response_headers;

protected:
	HttpConnectionBase(Source &source, Sink &sink) noexcept : source(source), sink(sink), limited_source(source, 0), chunked_source(source), chunked_sink(sink), read_source(), write_sink(), read_chunked(), response_headers_read() { }

public:
	void request(HttpRequestHeaders &request_headers);
	const HttpResponseHeaders & get_response_headers();
	ssize_t read(void *buf, size_t n) override;
	size_t write(const void *buf, size_t n) override;
	bool flush() override;

};


class HttpConnection : public HttpConnectionBase {

public:
	static constexpr const char *protocol_name = "http";

private:
	Socket socket;
	BufferedSource<1500> buffered_source;
	BufferedSink<1500> buffered_sink;

public:
	HttpConnection(Socket &&socket) noexcept;

};


#ifdef HTTP_TLS

class HttpsConnection : public HttpConnectionBase {

public:
	static constexpr const char *protocol_name = "https";

private:
	TLSSocket tls;

public:
	HttpsConnection(Socket &&socket, std::string host, const char ca_file[] = nullptr);

};

#endif // defined(HTTP_TLS)
