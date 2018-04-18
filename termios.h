#include <termios.h>


namespace posix {

static inline speed_t cfgetispeed(const struct termios &termios_r) noexcept { return ::cfgetispeed(&termios_r); }
static inline speed_t cfgetospeed(const struct termios &termios_r) noexcept { return ::cfgetospeed(&termios_r); }
void cfsetispeed(struct termios &termios_r, speed_t speed);
void cfsetospeed(struct termios &termios_r, speed_t speed);
void tcdrain(int fildes);
void tcflow(int fildes, int action);
void tcflush(int fildes, int queue_selector);
void tcgetattr(int fildes, struct termios &termios_r);
pid_t tcgetsid(int fildes);
void tcsendbreak(int fildes, int duration);
void tcsetattr(int fildes, int optional_actions, const struct termios &termios_r);

} // namespace posix
