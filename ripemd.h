#include "hash.h"


class RIPEMD160 : public Hash<64, 20, 20, uint32_t, uint64_t, false> {

public:
	RIPEMD160();

protected:
	void update(const uint8_t (&block)[64]) override;

};
