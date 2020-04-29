#include "hash.h"


class RIPEMD160 : public Hash<64, 20, 20, uint32_t, uint64_t, false> {

public:
	RIPEMD160() noexcept;

protected:
	void update(const std::byte (&block)[64]) override;

};
