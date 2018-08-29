#include "compiler.h"
#include "http.h"
#include "selector.h"
#include "socket.h"

namespace {
class Handshake;
}

class WebSocketServer : public Socket, public Selectable {
	friend Handshake;

public:
	using status_t = std::pair<unsigned, const char *>;

protected:
	using Socket::Socket;

protected:
	void selected(Selector &selector, Selector::Flags flags) noexcept override;
	virtual status_t validate_request_headers(const HttpRequestHeaders &request_headers) _pure;
	virtual void prepare_response_headers(const HttpRequestHeaders &, HttpResponseHeaders &) { }
	virtual void client_attached(Socket &&socket, Selector &selector, const HttpRequestHeaders &request_headers) = 0;

};
