#include "webclient.h"

#include <iostream>

#include "dns.h"


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


void HttpConnectionBase::request(HttpRequestHeaders &request_headers) {
#ifdef HTTP_GZIP
	if (request_headers.find_token("Accept-Encoding", "gzip") == request_headers.end()) {
		request_headers.emplace("Accept-Encoding", "gzip");
	}
#endif
	SinkBuf sb(sink);
	std::ostream os(&sb);
	os.exceptions(std::ios_base::badbit | std::ios_base::failbit);
	os << request_headers << std::flush;
	write_sink = request_headers.find("Content-Length") == request_headers.end() ? &chunked_sink : &sink;
	response_headers_read = false;
}

const HttpResponseHeaders & HttpConnectionBase::get_response_headers() {
	if (!response_headers_read) {
		DelimitedSource ds(source, "\r\n\r\n");
		SourceBuf sb(ds);
		char buf[1500];
		sb.pubsetbuf(buf, sizeof buf);
		std::istream(&sb) >> response_headers;
		if (read_chunked = response_headers.find_token("Transfer-Encoding", "chunked") != response_headers.end()) {
			chunked_source.reset();
			read_source = &chunked_source;
		}
		else {
			auto content_length_itr = response_headers.find("Content-Length");
			if (content_length_itr == response_headers.end()) {
				read_source = &source;
			}
			else {
				limited_source.remaining = std::stoul(content_length_itr->second);
				read_source = &limited_source;
			}
		}
#ifdef HTTP_GZIP
		if (response_headers.find_token("Content-Encoding", "gzip") != response_headers.end()) {
			gzip_source.emplace(*read_source);
			read_source = &*gzip_source;
		}
#endif
		response_headers_read = true;
	}
	return response_headers;
}

ssize_t HttpConnectionBase::read(void *buf, size_t n) {
	this->get_response_headers();
	ssize_t r;
	if ((r = read_source->read(buf, n)) < 0) {
		if (read_chunked) {
			read_chunked = false;
			limited_source.remaining = 0;
			read_source = &limited_source;
			SourceBuf sb(source);
			std::istream is(&sb);
			if (!skip_crlf(is)) {
				throw std::ios_base::failure("HTTP trailers not supported");
			}
		}
	}
	return r;
}

size_t HttpConnectionBase::write(const void *buf, size_t n) {
	return write_sink->write(buf, n);
}

bool HttpConnectionBase::flush() {
	return write_sink->flush();
}


constexpr const char *HttpConnection::protocol_name;

HttpConnection::HttpConnection(Socket &&socket) : HttpConnectionBase(buffered_source, buffered_sink), socket(std::move(socket)), buffered_source(this->socket), buffered_sink(this->socket) {
}


#ifdef HTTP_TLS

constexpr const char *HttpsConnection::protocol_name;

HttpsConnection::HttpsConnection(Socket &&socket, const std::string &host, const char ca_file[]) : HttpConnectionBase(tls, tls), tls(host, std::move(socket)) {
	tls.set_priority_direct("NORMAL");
	auto cred = std::make_shared<TLSCertificateCredentials>();
	if (ca_file) {
		cred->set_x509_trust_file(ca_file);
	}
	else {
		cred->set_x509_system_trust();
	}
	tls.set_credentials(cred);
	tls.handshake();
}

#endif // defined(HTTP_TLS)
