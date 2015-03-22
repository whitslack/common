#include "termios.h"

#include <system_error>


namespace posix {

void cfsetispeed(struct termios &termios_r, speed_t speed) {
	if (::cfsetispeed(&termios_r, speed) < 0) {
		throw std::system_error(errno, std::system_category(), "cfsetispeed");
	}
}

void cfsetospeed(struct termios &termios_r, speed_t speed) {
	if (::cfsetospeed(&termios_r, speed) < 0) {
		throw std::system_error(errno, std::system_category(), "cfsetospeed");
	}
}

void tcdrain(int fildes) {
	if (::tcdrain(fildes) < 0) {
		throw std::system_error(errno, std::system_category(), "tcdrain");
	}
}

void tcflow(int fildes, int action) {
	if (::tcflow(fildes, action) < 0) {
		throw std::system_error(errno, std::system_category(), "tcflow");
	}
}

void tcflush(int fildes, int queue_selector) {
	if (::tcflush(fildes, queue_selector) < 0) {
		throw std::system_error(errno, std::system_category(), "tcflush");
	}
}

void tcgetattr(int fildes, struct termios &termios_r) {
	if (::tcgetattr(fildes, &termios_r) < 0) {
		throw std::system_error(errno, std::system_category(), "tcgetattr");
	}
}

pid_t tcgetsid(int fildes) {
	pid_t ret;
	if ((ret = ::tcgetsid(fildes)) < 0) {
		throw std::system_error(errno, std::system_category(), "tcgetsid");
	}
	return ret;
}

void tcsendbreak(int fildes, int duration) {
	if (::tcsendbreak(fildes, duration) < 0) {
		throw std::system_error(errno, std::system_category(), "tcsendbreak");
	}
}

void tcsetattr(int fildes, int optional_actions, const struct termios &termios_r) {
	if (::tcsetattr(fildes, optional_actions, &termios_r) < 0) {
		throw std::system_error(errno, std::system_category(), "tcsetattr");
	}
}

} // namespace posix
