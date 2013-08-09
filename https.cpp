#include "https.h"

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
