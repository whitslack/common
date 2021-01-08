#include "uuid.h"

#include <iomanip>
#include <ostream>


std::ostream & operator<<(std::ostream &os, const UUID &uuid) {
	auto flags = os.flags(std::ios_base::hex | std::ios_base::right);
	auto fill = os.fill('0');
	os << std::setw(8) << uuid.fields.time_low <<
		'-' << std::setw(4) << uuid.fields.time_mid <<
		'-' << std::setw(4) << uuid.fields.time_hi_and_version <<
		'-' << std::setw(4) << uuid.fields.clock_seq_and_variant <<
		'-' << std::setw(2) << static_cast<unsigned>(uuid.fields.node[0]) <<
			std::setw(2) << static_cast<unsigned>(uuid.fields.node[1]) <<
			std::setw(2) << static_cast<unsigned>(uuid.fields.node[2]) <<
			std::setw(2) << static_cast<unsigned>(uuid.fields.node[3]) <<
			std::setw(2) << static_cast<unsigned>(uuid.fields.node[4]) <<
			std::setw(2) << static_cast<unsigned>(uuid.fields.node[5]);
	os.fill(fill);
	os.flags(flags);
	return os;
}
