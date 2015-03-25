#include "murmur3.h"


uint32_t murmur3_32(const void *data, size_t data_size, uint32_t seed) {
	uint32_t hash = seed;
	size_t data_rem = data_size;
	for (; data_rem >= sizeof(uint32_t); data_rem -= sizeof(uint32_t)) {
		hash = rotl(hash ^ rotl(letoh(*reinterpret_cast<const uint32_t *&>(data)++) * 0xcc9e2d51, 15) * 0x1b873593, 13) * 5 + 0xe6546b64;
	}
	uint32_t tail = 0;
	switch (data_rem) {
		case 3:
			tail |= static_cast<const uint8_t *>(data)[2] << 16;
		case 2:
			tail |= static_cast<const uint8_t *>(data)[1] << 8;
		case 1:
			tail |= static_cast<const uint8_t *>(data)[0];
			hash ^= rotl(tail * 0xcc9e2d51, 15) * 0x1b873593;
	}
	hash ^= static_cast<uint32_t>(data_size);
	hash = (hash ^ (hash >> 16)) * 0x85ebca6b;
	hash = (hash ^ (hash >> 13)) * 0xc2b2ae35;
	return hash ^ (hash >> 16);
}
