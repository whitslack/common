#include <libaio.h>


#undef linux
namespace linux {

class AIOContext {

private:
	io_context_t ctx;

public:
	explicit AIOContext(unsigned nr_events);
	~AIOContext();

public:
	size_t submit(size_t nr, struct iocb *iocbpp[]);
	void cancel(struct iocb *iocb, struct io_event *result);
	size_t getevents(size_t min_nr, size_t nr, struct io_event events[], struct timespec *timeout = nullptr);

};

} // namespace linux
