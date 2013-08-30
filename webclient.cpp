#include "webclient.h"

#include <iostream>


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


void HttpConnectionBase::request(const HttpRequestHeaders &request_headers) {
	SinkBuf sb(sink);
	std::ostream os(&sb);
	os.exceptions(std::ios_base::badbit | std::ios_base::failbit);
	os << request_headers << std::flush;
	write_chunked = request_headers.find("Content-Length") == request_headers.end();
	response_headers_read = false;
}

const HttpResponseHeaders & HttpConnectionBase::get_response_headers() {
	if (!response_headers_read) {
		DelimitedSource ds(source, "\r\n\r\n");
		SourceBuf sb(&ds);
		char buf[1500];
		sb.pubsetbuf(buf, sizeof buf);
		std::istream(&sb) >> response_headers;
		read_chunked = response_headers.find_token("Transfer-Encoding", "chunked") != response_headers.end();
		if (read_chunked) {
			chunked_source.reset();
		}
		else {
			auto content_length_itr = response_headers.find("Content-Length");
			remaining = content_length_itr == response_headers.end() ? ~0 : std::stoul(content_length_itr->second);
		}
		response_headers_read = true;
	}
	return response_headers;
}

ssize_t HttpConnectionBase::read(void *buf, size_t n) {
	this->get_response_headers();
	ssize_t r;
	if (read_chunked) {
		if ((r = chunked_source.read(buf, n)) < 0) {
			read_chunked = false;
			remaining = 0;
			SourceBuf sb(source);
			std::istream is(&sb);
			if (!skip_crlf(is)) {
				throw std::ios_base::failure("HTTP trailers not supported");
			}
		}
	}
	else {
		if (~remaining == 0) {
			r = source->read(buf, n);
		}
		else if (remaining == 0) {
			r = -1;
		}
		else if ((r = source->read(buf, std::min(n, remaining))) > 0) {
			remaining -= r;
		}
	}
	return r;
}

size_t HttpConnectionBase::write(const void *buf, size_t n, bool more) {
	return write_chunked ? chunked_sink.write(buf, n, more) : sink->write(buf, n, more);
}

bool HttpConnectionBase::finish() {
	return write_chunked ? chunked_sink.finish() : sink->finish();
}


HttpConnection::HttpConnection(const std::string &host, uint16_t port) : HttpConnectionBase(&buffered_source, &buffered_sink), buffered_source(&socket), buffered_sink(&socket) {
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


#ifdef HTTP_TLS

HttpsConnection::HttpsConnection(const std::string &host, uint16_t port, const char ca_file[]) : HttpConnectionBase(&tls, &tls) {
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
			Socket socket(info.ai_family, info.ai_socktype | SOCK_CLOEXEC, info.ai_protocol);
			int optval = 1;
			socket.setsockopt(SOL_SOCKET, SO_KEEPALIVE, &optval, static_cast<socklen_t>(sizeof optval));
			socket.connect(info.ai_addr, info.ai_addrlen);
			tls = TLSSocket(host, std::move(socket));
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
			return;
		}
		catch (...) {
			continue;
		}
	}
	throw std::system_error(errno, std::system_category(), "connect");
}

#endif // defined(HTTP_TLS)
