#include "progress.h"

#include <functional>
#include <iomanip>
#include <sstream>

#include <unistd.h>
#include <sys/ioctl.h>

static int get_cols() {
	struct winsize ws;
	return ::ioctl(2, TIOCGWINSZ, &ws) == 0 ? ws.ws_col : -1;
}

template <typename T>
static std::enable_if_t<std::is_unsigned_v<T>, std::ostream &> format_size(std::ostream &os, T size) {
	if (size < uint_least64_t(1000)) {
		os << size << " B";
	}
	else {
		char prefix;
		if (size < uint_least64_t(1000) << 10) {
			prefix = 'K';
		}
		else if (size < uint_least64_t(1000) << 20) {
			size >>= 10;
			prefix = 'M';
		}
		else if (size < uint_least64_t(1000) << 30) {
			size >>= 20;
			prefix = 'G';
		}
		else if (size < uint_least64_t(1000) << 40) {
			size >>= 30;
			prefix = 'T';
		}
		else if (size < uint_least64_t(1000) << 50) {
			size >>= 40;
			prefix = 'P';
		}
		else {
			size >>= 50;
			prefix = 'E';
		}
		os << (size >> 10);
		if (size < 10 << 10) {
			os << '.', os << (size = size * 10 >> 10) / 10, os << size % 10;
		}
		else if (size < 100 << 10) {
			os << '.' << (size / 10 >> 10);
		}
		os << ' ' << prefix << "iB";
	}
	return os;
}


ProgressBar::ProgressBar(uint_least64_t size) : pos(), size(size), finished() {
	int cols = get_cols();
	if (cols > 0) {
		display_thread = std::thread(std::mem_fn(&ProgressBar::display), this);
	}
}

ProgressBar::~ProgressBar() {
	finished = true;
	condition.notify_one();
	if (display_thread.joinable()) {
		display_thread.join();
	}
	char newline = '\n';
	if (::write(2, &newline, sizeof newline) < 0) {
	}
}

void ProgressBar::update(uint_least64_t delta) {
	std::lock_guard<std::mutex> lock(mutex);
	pos += delta;
	this->delta += delta;
}

void ProgressBar::display() {
	std::unique_lock<std::mutex> lock(mutex);
	while (!finished) {
		condition.wait_for(lock, std::chrono::seconds(1));
		delta >>= 1;
		int cols = get_cols();
		if (cols > 0) {
			std::ostringstream oss;
			format_size(format_size(format_size(oss << "  " << pos * 100 / size << '%' << "  ", pos) << " / ", size) << "  ", delta) << "/s";
			uint_least64_t rem = (size - pos) / delta;
			oss << std::setfill('0') << "  " << rem / 3600 << ':' << std::setw(2) << rem % 3600 / 60 << ':' << std::setw(2) << rem % 60 << " \r";
			std::string line = oss.str();
			if (static_cast<size_t>(cols) >= line.size()) {
				size_t bar_size = cols - line.size();
				line.insert(0, bar_size, ' ');
				if (bar_size >= 16) {
					line[2] = '[';
					line[bar_size - 1] = ']';
					bar_size = static_cast<size_t>((bar_size - 4) * pos / size);
					if (bar_size > 0) {
						line[2 + bar_size] = '>';
						--bar_size;
						line.replace(3, bar_size, bar_size, '=');
					}
				}
				if (::write(2, line.data(), line.size()) < 0) {
				}
			}
		}
	}
}
