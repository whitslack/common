#include "websocket.h"

#include <iostream>
#include <sstream>
#include <stdexcept>

#include "base64.h"
#include "codec.h"
#include "rawstrbuf.h"
#include "sha.h"

template <typename T, size_t N>
constexpr size_t countof(T (&)[N]) { return N; }

void WebSocketBase::send(WebSocket::Opcode opcode, bool mask, const void *buf, size_t n, bool more) {
	std::lock_guard<std::mutex> lock(send_mutex);
	uint8_t header[14], *p = header;
	*p++ = static_cast<uint8_t>(more ? opcode : 1 << 7 | opcode);
	if (n <= 125) {
		*p++ = static_cast<uint8_t>(mask ? 1 << 7 | n : n);
	}
	else if (n < 1 << 16) {
		*p++ = static_cast<uint8_t>(mask ? 1 << 7 | 126 : 126);
		*p++ = static_cast<uint8_t>(n >> 8);
		*p++ = static_cast<uint8_t>(n);
	}
	else {
		*p++ = static_cast<uint8_t>(mask ? 1 << 7 | 127 : 127);
#if SIZE_MAX >= UINT64_MAX
		*p++ = static_cast<uint8_t>(n >> 56);
		*p++ = static_cast<uint8_t>(n >> 48);
		*p++ = static_cast<uint8_t>(n >> 40);
		*p++ = static_cast<uint8_t>(n >> 32);
#else
		*p++ = 0, *p++ = 0, *p++ = 0, *p++ = 0;
#endif
		*p++ = static_cast<uint8_t>(n >> 24);
		*p++ = static_cast<uint8_t>(n >> 16);
		*p++ = static_cast<uint8_t>(n >> 8);
		*p++ = static_cast<uint8_t>(n);
	}
	if (mask) {
		uint32_t mask = static_cast<uint32_t>(::mrand48());
		*p++ = static_cast<uint8_t>(mask);
		*p++ = static_cast<uint8_t>(mask >> 8);
		*p++ = static_cast<uint8_t>(mask >> 16);
		*p++ = static_cast<uint8_t>(mask >> 24);
		std::vector<uint8_t> masked(n);
		for (size_t i = 0; i < n; ++i) {
			masked[i] = static_cast<const uint8_t *>(buf)[i] ^ static_cast<uint8_t>(mask >> i % 4 * 8);
		}
		this->send(header, p - header, true);
		this->send(masked.data(), masked.size(), more);
	}
	else {
		this->send(header, p - header, true);
		this->send(buf, n, more);
	}
}

void WebSocketBase::ready() {
	for (;;) {
		if (header_pos < 2) {
			if ((header_pos += this->recv(header_buf.data() + header_pos, 2 - header_pos)) < 2) {
				return;
			}
			data_rem = header_buf[1] & (1 << 7) - 1;
		}
		bool masked = static_cast<int8_t>(header_buf[1]) < 0;
		size_t header_len = masked ? 6 : 2, data_len = header_buf[1] & (1 << 7) - 1;
		if (data_len > 125) {
			if (data_len == 127) {
				throw std::length_error("received frame exceeds size limit");
			}
			header_len += 2;
			if (header_pos < header_len) {
				if ((header_pos += this->recv(header_buf.data() + header_pos, header_len - header_pos)) < header_len) {
					return;
				}
				data_rem = header_buf[2] << 8 | header_buf[3];
			}
		}
		else if (header_pos < header_len && (header_pos += this->recv(header_buf.data() + header_pos, header_len - header_pos)) < header_len) {
			return;
		}
		if (data_rem > 0) {
			size_t n = this->recv(data_buf.data() + data_pos, data_rem);
			if (n == 0) {
				return;
			}
			data_rem -= n;
			if (masked) {
				const uint8_t *mask = header_buf.data() + header_len - 4;
				do {
					data_buf[data_pos] ^= mask[data_pos % 4];
					++data_pos;
				} while (--n > 0);
			}
			else {
				data_pos += n;
			}
		}
		if (data_rem == 0) {
			size_t n = data_pos;
			data_pos = header_pos = 0;
			this->received(static_cast<Opcode>(header_buf[0] & (1 << 4) - 1), masked, data_buf.data(), n, static_cast<int8_t>(header_buf[0]) >= 0);
		}
	}
}


void WebSocket::send(const char text[], size_t n) {
	this->WebSocketBase::send(Text, this->is_client(), text, n);
}

void WebSocket::send(const void *buf, size_t n, bool more) {
	socket.write_fully(buf, n, more);
}

size_t WebSocket::recv(void *buf, size_t n) {
	ssize_t r;
	if ((r = socket.recv(buf, n)) < 0) {
		throw std::ios_base::failure("connection terminated");
	}
	return static_cast<size_t>(r);
}

void WebSocket::received(Opcode opcode, bool masked, void *buf, size_t n, bool more) {
	if (this->is_client()) {
		if (masked) {
			throw std::ios_base::failure("WebSocket frame from server was masked");
		}
	}
	else if (!masked) {
		throw std::ios_base::failure("WebSocket frame from client was not masked");
	}
	if (more) {
		throw std::ios_base::failure("fragmented message received");
	}
	switch (opcode) {
		case Text: {
			this->received(static_cast<char *>(buf), n);
			break;
		}
		case Close:
			if (n > 1) {
				this->WebSocketBase::send(Close, !masked, buf, 2, false);
			}
			else {
				this->WebSocketBase::send(Close, !masked, nullptr, 0, false);
			}
			socket.shutdown(SHUT_RDWR);
			break;
		case Ping:
			this->WebSocketBase::send(Pong, !masked, buf, n, false);
			break;
		default:
			throw std::ios_base::failure("unsupported opcode");
	}
}

WebSocket::operator int () const {
	return socket;
}

void WebSocket::ready(EPoll &epoll, uint32_t events) {
	if (events & EPOLLIN) {
		this->WebSocketBase::ready();
		epoll.watch(this, EPOLLIN);
	}
}


static std::string make_accept_field_value(const std::string &key) {
	SHA1 sha1;
	SinkBuf sink(&sha1);
	std::ostream(&sink) << key << "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
	auto &hash = sha1.finish();
	std::string str;
	StringSink ss(&str);
	CodecSink<Base64Encoder>(&ss).write(hash, sizeof hash);
	return str;
}

bool WebSocketServerHandshake::ready() {
	ssize_t r;
	if ((r = socket.recv(request_buf.data() + request_pos, request_buf.size() - request_pos)) < 0) {
		return false;
	}
	if ((request_pos += r) > 4 && ::memcmp(&request_buf[request_pos - 4], "\r\n\r\n", 4) == 0) {
		rawstrbuf rsb(reinterpret_cast<const char *>(request_buf.data()), request_pos);
		HttpRequestHeaders request_headers;
		if (std::istream(&rsb) >> request_headers) {
			auto error_pair = this->validate_request_headers(request_headers);
			if (error_pair.first >= 300) {
				this->send_error(error_pair.first, error_pair.second);
				throw std::ios_base::failure(error_pair.second);
			}
			auto host_itr = request_headers.find("Host");
			auto upgrade_itr = request_headers.find("Upgrade");
			auto connection_itr_pair = request_headers.split("Connection");
			auto key_itr = request_headers.find("Sec-WebSocket-Key");
			auto version_itr = request_headers.find("Sec-WebSocket-Version");
			auto end_itr = request_headers.end();
			if (host_itr != end_itr && upgrade_itr != end_itr && connection_itr_pair.first != connection_itr_pair.second && key_itr != end_itr && version_itr != end_itr && compare_ci(upgrade_itr->second, "websocket") == 0 && any_equal_ci(connection_itr_pair.first, connection_itr_pair.second, "Upgrade") && key_itr->second.size() == 24) {
				if (version_itr->second != "13") {
					HttpResponseHeaders response_headers("HTTP/1.1", 426, HTTP_REASON_PHRASE_426);
					// C++11: response_headers.emplace_hint(…)
					response_headers.insert(response_headers.end(), { "Connection", "close" });
					response_headers.insert(response_headers.end(), { "Sec-WebSocket-Version", "13" });
					SinkBuf sb(&socket);
					char buf[1024];
					sb.pubsetbuf(buf, sizeof buf);
					std::ostream(&sb) << response_headers << std::flush;
					throw std::ios_base::failure(HTTP_REASON_PHRASE_426);
				}
				HttpResponseHeaders response_headers("HTTP/1.1", error_pair.first, error_pair.second);
				// C++11: response_headers.emplace_hint(…)
				response_headers.insert(response_headers.end(), { "Connection", "Upgrade" });
				response_headers.insert(response_headers.end(), { "Sec-WebSocket-Accept", make_accept_field_value(key_itr->second) });
				response_headers.insert(response_headers.end(), { "Upgrade", "websocket" });
				this->prepare_response_headers(request_headers, response_headers);
				SinkBuf sb(&socket);
				char buf[1024];
				sb.pubsetbuf(buf, sizeof buf);
				std::ostream(&sb) << response_headers << std::flush;
				this->connected(request_headers, response_headers);
				return false;
			}
		}
		this->send_error(400, HTTP_REASON_PHRASE_400);
		throw std::ios_base::failure(HTTP_REASON_PHRASE_400);
	}
	if (request_pos == request_buf.size()) {
		this->send_error(431, HTTP_REASON_PHRASE_431);
		throw std::ios_base::failure(HTTP_REASON_PHRASE_431);
	}
	return true;
}

std::pair<uint, const char *> WebSocketServerHandshake::validate_request_headers(const HttpRequestHeaders &request_headers) {
	if (::strncasecmp(request_headers.method.data(), "GET", request_headers.method.size())) {
		return { 405, HTTP_REASON_PHRASE_405 };
	}
	if (::strncasecmp(request_headers.protocol_version.data(), "HTTP/1.1", request_headers.protocol_version.size())) {
		return { 505, HTTP_REASON_PHRASE_505 };
	}
	if (request_headers.request_uri != "/") {
		return { 404, HTTP_REASON_PHRASE_404 };
	}
	return { 101, HTTP_REASON_PHRASE_101 };
}

void WebSocketServerHandshake::prepare_response_headers(const HttpRequestHeaders &, HttpResponseHeaders &) {
}

void WebSocketServerHandshake::send_error(int status_code, const char reason_phrase[]) {
	HttpResponseHeaders response_headers("HTTP/1.1", status_code, reason_phrase);
	// C++11: response_headers.emplace_hint(…)
	response_headers.insert(response_headers.end(), { "Connection", "close" });
	response_headers.insert(response_headers.end(), { "Sec-WebSocket-Version", "13" });
	SinkBuf sb(&socket);
	char buf[1024];
	sb.pubsetbuf(buf, sizeof buf);
	std::ostream(&sb) << response_headers << std::flush;
}


void WebSocketClientHandshake::start(const char host[], uint16_t port, const char request_uri[]) {
	HttpRequestHeaders request_headers("GET", request_uri, "HTTP/1.1");
	request_headers.insert(request_headers.end(), { "Connection", "Upgrade" });
	if (port == 0) {
		request_headers.insert(request_headers.end(), { "Host", host });
	}
	else {
		std::ostringstream ss;
		ss << host << ':' << port;
		request_headers.insert(request_headers.end(), { "Host", ss.str() });
	}
	{
		uint32_t key[4];
		for (size_t i = 0; i < countof(key); ++i) {
			key[i] = static_cast<uint32_t>(::mrand48());
		}
		StringSink ss(&this->key);
		CodecSink<Base64Encoder>(&ss).write(key, sizeof key);
		request_headers.insert(request_headers.end(), { "Sec-WebSocket-Key", this->key });
	}
	request_headers.insert(request_headers.end(), { "Sec-WebSocket-Version", "13" });
	request_headers.insert(request_headers.end(), { "Upgrade", "websocket" });
	this->prepare_request_headers(request_headers);
	SinkBuf sb(&socket);
	char buf[1024];
	sb.pubsetbuf(buf, sizeof buf);
	std::ostream(&sb) << request_headers << std::flush;
}

bool WebSocketClientHandshake::ready() {
	ssize_t r;
	if ((r = socket.recv(response_buf.data() + response_pos, response_buf.size() - response_pos)) < 0) {
		return false;
	}
	if ((response_pos += r) > 4 && ::memcmp(&response_buf[response_pos - 4], "\r\n\r\n", 4) == 0) {
		rawstrbuf rsb(reinterpret_cast<const char *>(response_buf.data()), response_pos);
		HttpResponseHeaders response_headers;
		if (std::istream(&rsb) >> response_headers) {
			if (response_headers.status_code != 101) {
				throw std::ios_base::failure(response_headers.reason_phrase);
			}
			this->validate_response_headers(response_headers);
			auto upgrade_itr = response_headers.find("Upgrade");
			auto connection_itr_pair = response_headers.split("Connection");
			auto accept_itr = response_headers.find("Sec-WebSocket-Accept");
			auto end_itr = response_headers.end();
			if (upgrade_itr != end_itr && connection_itr_pair.first != connection_itr_pair.second && accept_itr != end_itr && compare_ci(upgrade_itr->second, "websocket") == 0 && any_equal_ci(connection_itr_pair.first, connection_itr_pair.second, "Upgrade") && accept_itr->second.size() == 28 && accept_itr->second == make_accept_field_value(key)) {
				this->connected(response_headers);
				return false;
			}
			throw std::ios_base::failure("WebSocket handshake failed");
		}
	}
	if (response_pos == response_buf.size()) {
		throw std::ios_base::failure("response headers too large");
	}
	return true;
}

void WebSocketClientHandshake::prepare_request_headers(HttpRequestHeaders &) {
}

void WebSocketClientHandshake::validate_response_headers(const HttpResponseHeaders &) {
}


WebSocketBuf::WebSocketBuf(WebSocket *ws, bool mask, WebSocket::Opcode opcode) : ws(ws), mask(mask), opcode(opcode) {
	this->setp(buf.data(), buf.data() + buf.size());
}

int WebSocketBuf::sync(bool more) {
	char_type *pbase = this->pbase(), *pptr = this->pptr();
	if (pptr > pbase) {
		ws->WebSocketBase::send(opcode, mask, pbase, pptr - pbase, more);
		opcode = WebSocket::Continuation;
		this->setp(buf.data(), buf.data() + buf.size());
	}
	return 0;
}

int WebSocketBuf::sync() {
	return this->sync(false);
}

WebSocketBuf::int_type WebSocketBuf::overflow(int_type ch) {
	this->sync(true);
	if (!traits_type::eq_int_type(ch, traits_type::eof())) {
		*this->pptr() = traits_type::to_char_type(ch);
		this->pbump(1);
	}
	return 0;
}
