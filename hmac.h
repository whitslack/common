#include "io.h"

template <typename Hash_Type>
class HMAC : public Sink {

public:
	typedef Hash_Type hash_type;

public:
	static constexpr size_t digest_size = hash_type::digest_size;

private:
	uint8_t secret[hash_type::block_size];
	hash_type hash;

public:
	HMAC(const void *key, size_t n);

public:
	size_t write(const void *buf, size_t n) override { return hash.write(buf, n); }
	bool flush() override { return hash.flush(); }
	const uint8_t (& digest())[digest_size];

};
