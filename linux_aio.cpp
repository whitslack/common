#include "linux_aio.h"

#include <system_error>

#include "compiler.h"


namespace linux {

AIOContext::AIOContext(unsigned nr_events) {
	ctx = nullptr;
	if (int error = ::io_setup(nr_events, &ctx); _unlikely(error < 0)) {
		throw std::system_error(-error, std::system_category(), "io_setup");
	}
}

AIOContext::~AIOContext() {
	if (int error = ::io_destroy(ctx); _unlikely(error < 0)) {
		throw std::system_error(-error, std::system_category(), "io_destroy");
	}
}

size_t AIOContext::submit(size_t nr, struct iocb *iocbpp[]) {
	int ret;
	if (_unlikely((ret = ::io_submit(ctx, static_cast<long>(nr), iocbpp)) < 0)) {
		throw std::system_error(-ret, std::system_category(), "io_submit");
	}
	return static_cast<size_t>(ret);
}

void AIOContext::cancel(struct iocb *iocb, struct io_event *result) {
	if (int error = ::io_cancel(ctx, iocb, result); _unlikely(error < 0)) {
		throw std::system_error(-error, std::system_category(), "io_cancel");
	}
}

size_t AIOContext::getevents(size_t min_nr, size_t nr, struct io_event events[], struct timespec *timeout) {
	int ret;
	if (_unlikely((ret = ::io_getevents(ctx, static_cast<long>(min_nr), static_cast<long>(nr), events, timeout)) < 0)) {
		throw std::system_error(-ret, std::system_category(), "io_getevents");
	}
	return static_cast<size_t>(ret);
}

} // namespace linux
