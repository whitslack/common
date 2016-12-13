#include <array>
#include <cstdint>
#include <streambuf>
#include <experimental/optional>

#include "compiler.h"
#include "http.h"
#include "socket.h"

namespace stdx = std::experimental;


class WebSocket {

public:
	enum Opcode {
		Continuation = 0x0, Text = 0x1, Binary = 0x2,
		Close = 0x8, Ping = 0x9, Pong = 0xA
	};

public:
	Socket socket;

protected:
	std::array<uint8_t, 14> header_buf;
	size_t header_pos;
	std::array<uint8_t, 1 << 16> data_buf;
	size_t data_pos, data_rem;

public:
	WebSocket(Socket &&socket) : socket(std::move(socket)), header_pos(), data_pos() { }

public:
	void send(Opcode opcode, bool mask, const void *buf, size_t n, bool more = false);
	stdx::optional<MemorySource> receive(Opcode &opcode, bool masked);

private:
	void send(const void *buf, size_t n, bool more);
	size_t recv(void *buf, size_t n);

};


class WebSocketServerHandshake {

public:
	Socket socket;

private:
	std::array<uint8_t, 1460> request_buf;
	size_t request_pos;

public:
	WebSocketServerHandshake(Socket &&socket) : socket(std::move(socket)), request_pos() { }

public:
	bool ready();

protected:
	virtual std::pair<unsigned, const char *> validate_request_headers(const HttpRequestHeaders &request_headers);
	virtual void prepare_response_headers(const HttpRequestHeaders &request_headers, HttpResponseHeaders &response_headers) _const;
	virtual void connected(const HttpRequestHeaders &request_headers, const HttpResponseHeaders &response_headers) = 0;

private:
	void send_error(int status_code, const char reason_phrase[]);

};


class WebSocketClientHandshake {

public:
	Socket socket;

private:
	std::string key;
	DelimitedSource delimited_source;
	std::array<uint8_t, 1460> response_buf;
	size_t response_pos;

public:
	WebSocketClientHandshake(Socket &&socket) : socket(std::move(socket)), delimited_source(this->socket, "\r\n\r\n"), response_pos() { }

public:
	void start(const char host[], in_port_t port = 0, const char request_uri[] = "/");
	bool ready();

protected:
	virtual void prepare_request_headers(HttpRequestHeaders &request_headers) _const;
	virtual void validate_response_headers(const HttpResponseHeaders &response_headers) _const;
	virtual void connected(const HttpResponseHeaders &response_headers) = 0;

};


class WebSocketBuf : public std::streambuf {

protected:
	WebSocket * const ws;
	bool mask;
	WebSocket::Opcode opcode;
	std::array<char_type, 1 << 12> buf;

public:
	WebSocketBuf(WebSocket *ws, bool mask = false, WebSocket::Opcode opcode = WebSocket::Text);

protected:
	virtual int sync(bool more);
	int sync() override final;
	int_type overflow(int_type ch) override;

};


Socket connect_websocket(const char host[], in_port_t port, const char request_uri[], const char origin_uri[] = nullptr);
