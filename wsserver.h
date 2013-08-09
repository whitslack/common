#include "compiler.h"
#include "nbio.h"

namespace {
class Handshake;
}

class WebSocketServer : public Socket6, public EPollable {
	friend Handshake;

protected:
	// [C++11] using Socket6::Socket6;

protected:
	operator int () const override _pure;
	void ready(EPoll &epoll, uint32_t events) override;
	virtual void client_attached(Socket6 &&socket, EPoll &epoll) = 0;

};
