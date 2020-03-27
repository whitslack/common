#include <sys/eventfd.h>

#include "fd.h"


#undef linux
namespace linux {


class EventFD : public FileDescriptor {

public:
	explicit EventFD(unsigned int initval = 0, int flags = EFD_CLOEXEC);

public:
	eventfd_t read();
	_nodiscard bool write(eventfd_t value);

};


} // namespace linux
