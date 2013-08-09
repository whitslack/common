#include "http.h"
#include "tls.h"

class HttpsConnection : public HttpConnectionBase {

private:
	TLSSocket tls;

public:
	HttpsConnection(const std::string &host, uint16_t port = 443, const char ca_file[] = nullptr);

};
