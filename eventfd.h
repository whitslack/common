#include <sys/eventfd.h>

#include "fd.h"


class EventFD : public FileDescriptor {

public:
	explicit EventFD(unsigned int initval = 0, int flags = EFD_CLOEXEC);

public:
	eventfd_t read();
	void write(eventfd_t value);

};
