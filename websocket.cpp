#include "websocket.h"

#include <bit>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string_view>

#include "base64.h"
#include "codec.h"
#include "connect.h"
#include "endian.h"
#include "memory.h"
#include "sha.h"

using namespace ci::literals;


static void memxor32(void *buf, size_t n, uint32_t mask, size_t phase) {
#ifdef __GNUC__
	typedef uint32_t vec_t __attribute__ ((vector_size (16)));
#else
	typedef uint32_t vec_t;
#endif
	union {
		void *void_ptr;
		uint8_t *uint8_ptr;
		vec_t *vec_ptr;
		uintptr_t uintptr;
	} ptr;
	ptr.void_ptr = buf;
	uint32_t mask_le = std::rotr(+as_le(mask), unsigned(phase * 8));
	while (n > 0 && ptr.uintptr % sizeof(vec_t)) {
		*ptr.uint8_ptr = static_cast<uint8_t>(*ptr.uint8_ptr ^ mask_le), ++ptr.uint8_ptr, --n;
		mask_le = std::rotr(mask_le, 8);
	}
	uint32_t mask_he = as_le(mask_le);
	while (n >= sizeof(vec_t)) {
		*ptr.vec_ptr ^= mask_he, ++ptr.vec_ptr, n -= sizeof(vec_t);
	}
	while (n > 0) {
		*ptr.uint8_ptr = static_cast<uint8_t>(*ptr.uint8_ptr ^ mask_le), ++ptr.uint8_ptr, --n;
		mask_le = std::rotr(mask_le, 8);
	}
}


ssize_t WebSocket::receive(Opcode &opcode, void *buf, size_t n) {
	if (static_cast<int8_t>(recv_hdr_pos) >= 0) {
		if (recv_hdr_pos == 0) {
			ssize_t r = socket.read(&recv_state, sizeof recv_state + sizeof recv_hdr_pos);
			if (r <= 0) {
				if (r < 0) {
					opcode = End;
					return r;
				}
				return 0;
			}
			if (_unlikely(recv_state & 0x70)) {
				throw std::ios_base::failure("received WebSocket frame with non-zero reserved bits");
			}
			opcode = static_cast<Opcode>(recv_state & 0xF);
			switch (opcode) {
				case Continuation:
				case Text:
				case Binary:
					break;
				case Close:
				case Ping:
				case Pong:
					if (_unlikely(!(recv_state & 0x80))) {
						throw std::ios_base::failure("received fragmented WebSocket control frame");
					}
					break;
				default:
					throw std::ios_base::failure("received WebSocket frame with unrecognized opcode");
			}
			if (r == 1) {
				recv_hdr_pos = 1;
				return 0;
			}
			recv_mask = recv_hdr_pos & 0x80;
			recv_state = static_cast<uint8_t>(recv_hdr_pos & 0x7F | recv_state & 0x80);
			recv_hdr_pos = 2;
		}
		else if (recv_hdr_pos == 1) {
			ssize_t r = socket.read(&recv_hdr_pos, sizeof recv_hdr_pos);
			if (r <= 0) {
				if (r < 0) {
					opcode = End;
					return r;
				}
				return 0;
			}
			recv_mask = recv_hdr_pos & 0x80;
			recv_state = static_cast<uint8_t>(recv_hdr_pos & 0x7F | recv_state & 0x80);
			recv_hdr_pos = 2;
		}
		void *hdr_buf = &recv_mask;
		size_t hdr_size = 2, payload_len = recv_state & 0x7F;
		if (payload_len == 127) {
			hdr_buf = static_cast<uint8_t *>(hdr_buf) - sizeof recv_data_rem;
			hdr_size += sizeof recv_data_rem;
		}
		else if (payload_len == 126) {
			hdr_buf = static_cast<uint8_t *>(hdr_buf) - sizeof(uint16_t);
			hdr_size += sizeof(uint16_t);
		}
		if (recv_hdr_pos > hdr_size || recv_mask) {
			hdr_size += sizeof recv_mask;
		}
		if ((hdr_size -= recv_hdr_pos) > 0) {
			ssize_t r = socket.read(static_cast<uint8_t *>(hdr_buf) - 2 + recv_hdr_pos, hdr_size);
			if (r <= 0) {
				if (r < 0) {
					opcode = End;
					return r;
				}
				return 0;
			}
			recv_hdr_pos = static_cast<uint8_t>(recv_hdr_pos + r);
			if (static_cast<size_t>(r) < hdr_size) {
				return 0;
			}
		}
		if (payload_len > 125) {
			recv_data_rem = as_be(recv_data_rem);
		}
		else {
			recv_data_rem = payload_len;
		}
		recv_hdr_pos = ~0;
	}
	if (recv_data_rem == 0) {
		recv_hdr_pos = 0;
		return -1; // end of frame
	}
	if (n == 0) {
		return 0;
	}
	ssize_t r = socket.read(buf, std::min(n, recv_data_rem));
	if (r <= 0) {
		if (r < 0) {
			opcode = End;
			return r;
		}
		return 0;
	}
	recv_data_rem -= r;
	if (recv_mask) {
		memxor32(buf, r, recv_mask, ~recv_hdr_pos);
		recv_hdr_pos = static_cast<uint8_t>(~((~recv_hdr_pos + r) % sizeof recv_mask));
	}
	return r;
}

bool WebSocket::send(Opcode opcode, const void *buf, size_t n, bool more) {
	if (static_cast<int8_t>(send_hdr_pos) >= 0) {
		uint8_t send_hdr[14];
		size_t send_hdr_len = 2;
		send_hdr[0] = opcode & 0xF;
		if (!more) {
			send_hdr[0] |= 0x80;
		}
		if (n >> 16) {
			send_hdr[1] = 127;
#if SIZE_MAX > UINT32_MAX
			send_hdr[2] = static_cast<uint8_t>(n >> 56);
			send_hdr[3] = static_cast<uint8_t>(n >> 48);
			send_hdr[4] = static_cast<uint8_t>(n >> 40);
			send_hdr[5] = static_cast<uint8_t>(n >> 32);
#else
			send_hdr[5] = send_hdr[4] = send_hdr[3] = send_hdr[2] = 0;
#endif
			send_hdr[6] = static_cast<uint8_t>(n >> 24);
			send_hdr[7] = static_cast<uint8_t>(n >> 16);
			send_hdr[8] = static_cast<uint8_t>(n >> 8);
			send_hdr[9] = static_cast<uint8_t>(n);
			send_hdr_len += 8;
		}
		else if (n > 125) {
			send_hdr[1] = 126;
			send_hdr[2] = static_cast<uint8_t>(n >> 8);
			send_hdr[3] = static_cast<uint8_t>(n);
			send_hdr_len += 2;
		}
		else {
			send_hdr[1] = static_cast<uint8_t>(n);
		}
		if (send_mask) {
			send_hdr[1] |= 0x80;
			// always send a null mask, as this lets us avoid XORing the data
			std::memset(send_hdr + send_hdr_len, 0, sizeof(uint32_t));
			send_hdr_len += sizeof(uint32_t);
		}
		size_t w = socket.write({ { send_hdr + send_hdr_pos, send_hdr_len -= send_hdr_pos }, { buf, send_data_rem = n } });
		if (w < send_hdr_len) {
			send_hdr_pos = static_cast<uint8_t>(send_hdr_pos + w);
			return false;
		}
		w -= send_hdr_len;
		if ((send_data_rem -= w) > 0) {
			send_hdr_pos = ~0;
			return false;
		}
	}
	else {
		if ((send_data_rem -= socket.write(static_cast<const uint8_t *>(buf) + n - send_data_rem, send_data_rem)) > 0) {
			return false;
		}
	}
	if (!more) {
		socket.flush();
	}
	return true;
}


static std::string make_accept_field_value(std::string_view key) {
	SHA1 sha1;
	sha1.write_fully(key.data(), key.size());
	sha1.write_fully("258EAFA5-E914-47DA-95CA-C5AB0DC85B11", 36);
	auto &hash = sha1.digest();
	return transcode<Base64Encoder>(hash.data(), hash.size());
}

bool WebSocketServerHandshake::ready() {
	ssize_t r;
	if ((r = socket.read(request_buf.data() + request_pos, request_buf.size() - request_pos)) < 0) {
		return false;
	}
	if ((request_pos += r) > 4 && ::memcmp(&request_buf[request_pos - 4], "\r\n\r\n", 4) == 0) {
		HttpRequestHeaders request_headers;
		MemoryBuf mb(request_buf.data(), request_pos);
		if (std::istream(&mb) >> request_headers) {
			auto error_pair = this->validate_request_headers(request_headers);
			if (_unlikely(error_pair.first >= 300)) {
				this->send_error(error_pair.first, error_pair.second);
				throw std::ios_base::failure(error_pair.second);
			}
			auto host_itr = request_headers.find("Host"_ci);
			auto upgrade_itr = request_headers.find("Upgrade"_ci);
			auto connection_itr = request_headers.find_token("Connection", "Upgrade");
			auto key_itr = request_headers.find("Sec-WebSocket-Key"_ci);
			auto version_itr = request_headers.find("Sec-WebSocket-Version"_ci);
			auto end_itr = request_headers.end();
			if (host_itr != end_itr && upgrade_itr != end_itr && connection_itr != end_itr && key_itr != end_itr && version_itr != end_itr && upgrade_itr->second == "websocket"_ci && key_itr->second.size() == 24) {
				if (version_itr->second != "13") {
					HttpResponseHeaders response_headers("HTTP/1.1", 426, HTTP_REASON_PHRASE_426);
					response_headers.emplace_hint(response_headers.end(), "Connection", "close");
					response_headers.emplace_hint(response_headers.end(), "Sec-WebSocket-Version", "13");
					SinkBuf sb(socket);
					char buf[1024];
					sb.pubsetbuf(buf, sizeof buf);
					std::ostream(&sb) << response_headers << std::flush;
					throw std::ios_base::failure(HTTP_REASON_PHRASE_426);
				}
				HttpResponseHeaders response_headers("HTTP/1.1", error_pair.first, error_pair.second);
				response_headers.emplace_hint(response_headers.end(), "Connection", "Upgrade");
				response_headers.emplace_hint(response_headers.end(), "Sec-WebSocket-Accept", make_accept_field_value(key_itr->second));
				response_headers.emplace_hint(response_headers.end(), "Upgrade", "websocket");
				this->prepare_response_headers(request_headers, response_headers);
				SinkBuf sb(socket);
				char buf[1024];
				sb.pubsetbuf(buf, sizeof buf);
				std::ostream os(&sb);
				os.exceptions(std::ios_base::badbit | std::ios_base::failbit);
				os << response_headers << std::flush;
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

auto WebSocketServerHandshake::validate_request_headers(const HttpRequestHeaders &request_headers) -> status_t {
	if (request_headers.method != "GET"_ci) {
		return { 405, HTTP_REASON_PHRASE_405 };
	}
	if (request_headers.protocol_version != "HTTP/1.1"_ci) {
		return { 505, HTTP_REASON_PHRASE_505 };
	}
	return { 101, HTTP_REASON_PHRASE_101 };
}

void WebSocketServerHandshake::prepare_response_headers(const HttpRequestHeaders &, HttpResponseHeaders &) {
}

void WebSocketServerHandshake::send_error(int status_code, const char reason_phrase[]) {
	HttpResponseHeaders response_headers("HTTP/1.1", status_code, reason_phrase);
	response_headers.emplace_hint(response_headers.end(), "Connection", "close");
	response_headers.emplace_hint(response_headers.end(), "Sec-WebSocket-Version", "13");
	SinkBuf sb(socket);
	char buf[1024];
	sb.pubsetbuf(buf, sizeof buf);
	std::ostream os(&sb);
	os.exceptions(std::ios_base::badbit | std::ios_base::failbit);
	os << response_headers << std::flush;
}


void WebSocketClientHandshake::start(const char host[], in_port_t port, const char request_uri[]) {
	HttpRequestHeaders request_headers("GET", request_uri, "HTTP/1.1");
	request_headers.emplace_hint(request_headers.end(), "Connection", "Upgrade");
	if (port == 0) {
		request_headers.emplace_hint(request_headers.end(), "Host", host);
	}
	else {
		std::ostringstream ss;
		ss << host << ':' << port;
		request_headers.emplace_hint(request_headers.end(), "Host", ss.str());
	}
	{
		uint32_t key[4];
		for (size_t i = 0; i < std::size(key); ++i) {
			key[i] = static_cast<uint32_t>(::mrand48());
		}
		transcode<Base64Encoder>(this->key, key, sizeof key);
		request_headers.emplace_hint(request_headers.end(), "Sec-WebSocket-Key", this->key);
	}
	request_headers.emplace_hint(request_headers.end(), "Sec-WebSocket-Version", "13");
	request_headers.emplace_hint(request_headers.end(), "Upgrade", "websocket");
	this->prepare_request_headers(request_headers);
	SinkBuf sb(socket);
	char buf[1024];
	sb.pubsetbuf(buf, sizeof buf);
	std::ostream os(&sb);
	os.exceptions(std::ios_base::badbit | std::ios_base::failbit);
	os << request_headers << std::flush;
}

bool WebSocketClientHandshake::ready() {
	ssize_t r;
	if ((r = delimited_source.read(response_buf.data() + response_pos, response_buf.size() - response_pos)) < 0) {
		return false;
	}
	if ((response_pos += r) > 4 && ::memcmp(&response_buf[response_pos - 4], "\r\n\r\n", 4) == 0) {
		HttpResponseHeaders response_headers;
		MemoryBuf mb(response_buf.data(), response_pos);
		if (std::istream(&mb) >> response_headers) {
			if (_unlikely(response_headers.status_code != 101)) {
				throw std::ios_base::failure(response_headers.reason_phrase);
			}
			this->validate_response_headers(response_headers);
			auto upgrade_itr = response_headers.find("Upgrade"_ci);
			auto connection_itr = response_headers.find_token("Connection", "Upgrade");
			auto accept_itr = response_headers.find("Sec-WebSocket-Accept"_ci);
			auto end_itr = response_headers.end();
			if (upgrade_itr != end_itr && connection_itr != end_itr && accept_itr != end_itr && upgrade_itr->second == "websocket"_ci && accept_itr->second.size() == 28 && accept_itr->second == make_accept_field_value(key)) {
				this->connected(response_headers);
				return false;
			}
			throw std::ios_base::failure("WebSocket handshake failed");
		}
	}
	if (_unlikely(response_pos == response_buf.size())) {
		throw std::ios_base::failure("response headers too large");
	}
	return true;
}

void WebSocketClientHandshake::prepare_request_headers(HttpRequestHeaders &) {
}

void WebSocketClientHandshake::validate_response_headers(const HttpResponseHeaders &) {
}


WebSocketBuf::WebSocketBuf(WebSocket *ws, WebSocket::Opcode opcode) noexcept : ws(ws), opcode(opcode) {
	this->setp(buf.data(), buf.data() + buf.size());
}

int WebSocketBuf::sync(bool more) {
	char_type *pbase = this->pbase(), *pptr = this->pptr();
	if (pptr > pbase) {
		if (!ws->send(opcode, pbase, pptr - pbase, more)) {
			return -1;
		}
		opcode = WebSocket::Continuation;
		this->setp(buf.data(), buf.data() + buf.size());
	}
	return 0;
}

int WebSocketBuf::sync() {
	return this->sync(false);
}

WebSocketBuf::int_type WebSocketBuf::overflow(int_type ch) {
	if (this->sync(true) < 0) {
		return traits_type::eof();
	}
	if (!traits_type::eq_int_type(ch, traits_type::eof())) {
		*this->pptr() = traits_type::to_char_type(ch);
		this->pbump(1);
	}
	return 0;
}


Socket connect_websocket(const char host[], in_port_t port, const char request_uri[], const char origin_uri[]) {
	class Handshake : public WebSocketClientHandshake {

	public:
		bool finished;

	private:
		const char *origin_uri;

	public:
		Handshake(Socket &&socket, const char origin_uri[]) : WebSocketClientHandshake(std::move(socket)), finished(), origin_uri(origin_uri) { }

	protected:
		void prepare_request_headers(HttpRequestHeaders &request_headers) override {
			if (origin_uri) {
				request_headers.emplace("Origin", origin_uri);
			}
		}

		void connected(const HttpResponseHeaders &) override {
			finished = true;
		}

	} handshake(connect(host, port), origin_uri);
	handshake.start(host, port, request_uri);
	while (!handshake.finished) {
		handshake.ready();
	}
	return std::move(handshake.socket);
}
