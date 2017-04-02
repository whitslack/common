#include "compiler.h"
#include "http.h"
#include "selector.h"
#include "socket.h"

namespace {
class Handshake;
}

class WebSocketServer : public Socket, public Selectable {
	friend Handshake;

protected:
	using Socket::Socket;

protected:
	void selected(Selector &selector, Selector::Flags flags) noexcept override;
	virtual void prepare_response_headers(const HttpRequestHeaders &, HttpResponseHeaders &) { }
	virtual void client_attached(Socket &&socket, Selector &selector) = 0;

};
