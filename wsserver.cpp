#include "wsserver.h"

#include <cstring>
#include <iostream>
#include <sstream>

#include <netinet/tcp.h>

#include "base64.h"
#include "sha.h"
#include "websocket.h"

namespace {
class Handshake : public WebSocketServerHandshake, public Selectable {

private:
	WebSocketServer &server;
	Selector &selector;

public:
	Handshake(Socket &&socket, WebSocketServer &server, Selector &selector, bool add = true) : WebSocketServerHandshake(std::move(socket)), server(server), selector(selector) {
		if (add) {
			selector.add(this->socket, this, Selector::Flags::READABLE);
		}
		else {
			selector.modify(this->socket, this, Selector::Flags::READABLE);
		}
	}

public:
	void selected(Selector &selector, Selector::Flags flags) override {
		if ((flags & Selector::Flags::READABLE) != Selector::Flags::NONE) {
			try {
				if (this->WebSocketServerHandshake::ready()) {
					selector.modify(socket, this, Selector::Flags::READABLE);
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

protected:
	void prepare_response_headers(const HttpRequestHeaders &request_headers, HttpResponseHeaders &response_headers) override {
		server.prepare_response_headers(request_headers, response_headers);
	}

	void connected(const HttpRequestHeaders &, const HttpResponseHeaders &) override {
		server.client_attached(std::move(socket), selector);
	}

};
}

void WebSocketServer::selected(Selector &selector, Selector::Flags flags) {
	if ((flags & Selector::Flags::READABLE) != Selector::Flags::NONE) {
		Socket socket = this->accept(nullptr, nullptr, SOCK_NONBLOCK | SOCK_CLOEXEC);
		socket.setsockopt(SOL_SOCKET, SO_KEEPALIVE, 1);
		new Handshake(std::move(socket), *this, selector);
	}
	selector.modify(*this, this, Selector::Flags::READABLE);
}
