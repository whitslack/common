#include "nbio.h"

Socket connect(const char host[], uint16_t port);

Socket connect_with_retry(const char host[], uint16_t port);
