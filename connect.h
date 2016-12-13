#include <chrono>

#include "socket.h"


Socket connect(const char host[], in_port_t port, std::chrono::microseconds timeout = std::chrono::microseconds::zero());

Socket connect_with_retry(const char host[], in_port_t port);
