#include "wsserver.h"

#include <cstring>
#include <iostream>
#include <sstream>

#include <netinet/tcp.h>

#include "base64.h"
#include "sha.h"
#include "websocket.h"

namespace {
class Handshake : public WebSocketServerHandshake, public EPollable {

private:
	WebSocketServer &server;
	EPoll &epoll;

public:
	Handshake(Socket &&socket, WebSocketServer &server, EPoll &epoll, int epoll_op = EPOLL_CTL_ADD) : WebSocketServerHandshake(std::move(socket)), server(server), epoll(epoll) {
		epoll.watch(this, EPOLLIN, epoll_op);
	}

protected:
	operator int () const override _pure { return socket; }

	void ready(EPoll &epoll, uint32_t events) override {
		if (events & EPOLLIN) {
			try {
				if (this->WebSocketServerHandshake::ready()) {
					epoll.watch(this, EPOLLIN);
				}
				else {
					delete this;
				}
			}
			catch (...) {
				delete this;
			}
		}
	}

	void prepare_response_headers(const HttpRequestHeaders &request_headers, HttpResponseHeaders &response_headers) override {
		server.prepare_response_headers(request_headers, response_headers);
	}

	void connected(const HttpRequestHeaders &, const HttpResponseHeaders &) override {
		server.client_attached(std::move(*static_cast<Socket6 *>(&socket)), epoll);
	}

};
}

WebSocketServer::operator int () const {
	return fd;
}

void WebSocketServer::ready(EPoll &epoll, uint32_t events) {
	if ((events & EPOLLIN) != 0) {
		sockaddr_in6 from_addr;
		Socket6 socket = this->accept(&from_addr);
		const int optval = 1;
		socket.setsockopt(SOL_SOCKET, SO_KEEPALIVE, &optval, static_cast<socklen_t>(sizeof optval));
		socket.setsockopt(IPPROTO_TCP, TCP_NODELAY, &optval, static_cast<socklen_t>(sizeof optval));
		new Handshake(std::move(socket), *this, epoll);
	}
	epoll.watch(this, EPOLLIN);
}
