#include "connect.h"

#include <chrono>
#include <thread>

#include "log.h"

extern Log elog;

Socket connect(const char host[], uint16_t port, int send_buffer_size) {
	for (auto &info : getaddrinfo(host)) {
		if (info.ai_family == AF_INET) {
			reinterpret_cast<sockaddr_in *>(info.ai_addr)->sin_port = htobe16(port);
			if (elog.trace_enabled()) {
				elog.trace() << "connecting to " << host << " at " << *reinterpret_cast<sockaddr_in *>(info.ai_addr) << std::endl;
			}
		}
		else if (info.ai_family == AF_INET6) {
			reinterpret_cast<sockaddr_in6 *>(info.ai_addr)->sin6_port = htobe16(port);
			if (elog.trace_enabled()) {
				elog.trace() << "connecting to " << host << " at " << *reinterpret_cast<sockaddr_in6 *>(info.ai_addr) << std::endl;
			}
		}
		else {
			continue;
		}
		try {
			Socket socket(info.ai_family, info.ai_socktype | SOCK_CLOEXEC, info.ai_protocol);
			int optval = 1;
			socket.setsockopt(SOL_SOCKET, SO_KEEPALIVE, &optval, static_cast<socklen_t>(sizeof optval));
			if (send_buffer_size >= 0) {
				socket.setsockopt(SOL_SOCKET, SO_SNDBUF, &send_buffer_size, static_cast<socklen_t>(sizeof send_buffer_size));
			}
			socket.connect(info.ai_addr, info.ai_addrlen);
			if (elog.debug_enabled()) {
				if (info.ai_family == AF_INET) {
					sockaddr_in addr;
					socklen_t addrlen = static_cast<socklen_t>(sizeof addr);
					socket.getpeername(reinterpret_cast<sockaddr *>(&addr), &addrlen);
					elog.debug() << "connected to " << host << " at " << addr << std::endl;
				}
				else if (info.ai_family == AF_INET6) {
					sockaddr_in6 addr;
					socklen_t addrlen = static_cast<socklen_t>(sizeof addr);
					socket.getpeername(reinterpret_cast<sockaddr *>(&addr), &addrlen);
					elog.debug() << "connected to " << host << " at " << addr << std::endl;
				}
			}
			return socket;
		}
		catch (...) {
			continue;
		}
	}
	throw std::system_error(errno, std::system_category(), "connect");
}

Socket connect_with_retry(const char host[], uint16_t port, int send_buffer_size) {
	static const std::chrono::steady_clock::duration
			min_delay = std::chrono::milliseconds(500),
			max_delay = std::chrono::seconds(15);
	auto reconnect_delay = min_delay;
	for (;;) {
		try {
			return connect(host, port, send_buffer_size);
		}
		/*catch (const std::system_error &) {
			if (elog.warn_enabled()) {
				elog.warn() << "failed to connect to " << host << "; will retry" << std::endl;
			}
		}*/
		catch (const std::exception &e) {
			if (elog.warn_enabled()) {
				elog.warn() << "failed to connect to " << host << ": " << e.what() << "; will retry" << std::endl;
			}
		}
		std::this_thread::sleep_for(reconnect_delay);
		reconnect_delay = std::min(reconnect_delay + reconnect_delay / 2, max_delay);
	}
}
