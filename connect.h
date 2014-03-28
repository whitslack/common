#include "socket.h"

Socket connect(const char host[], uint16_t port, int send_buffer_size = -1);

Socket connect_with_retry(const char host[], uint16_t port, int send_buffer_size = -1);
