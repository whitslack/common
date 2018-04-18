#include <cstddef>
#include <cstdint>

#include "compiler.h"


uint32_t murmur3_32(const void *data, size_t data_size, uint32_t seed) noexcept _pure;
