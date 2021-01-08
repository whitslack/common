#include <algorithm>
#include <array>
#include <climits>
#include <functional>
#include <iosfwd>
#include <stdexcept>

#include "compiler.h"
#include "endian.h"
#include "span.h"


class UUID {

public:
	enum class Variant : uint8_t {
		NCS = 0, // 0xx
		DCE = 4, // 10x
		MICROSOFT = 6, // 110
	};

	enum class Version : uint8_t {
		TIME_BASED = 1,
		DCE_SECURITY = 2,
		NAME_BASED_MD5 = 3,
		RANDOM = 4,
		NAME_BASED_SHA1 = 5,
	};

	struct Fields {
		be<uint32_t> time_low;
		be<uint16_t> time_mid;
		be<uint16_t> time_hi_and_version;
		be<uint16_t> clock_seq_and_variant;
		std::array<std::byte, 6> node;
	};

public:
	static UUID generate_random();
	static UUID generate_time_based();

public:
	union {
#ifdef __SIZEOF_INT128__
		be<unsigned __int128> wide;
#endif
		std::array<unsigned long, 128 / ULONG_WIDTH> words;
		std::array<std::byte, 16> bytes;
		Fields fields;
	};

public:
	constexpr UUID() noexcept = default;
	constexpr UUID(std::nullptr_t) noexcept : bytes { } { }
	constexpr explicit UUID(const std::array<std::byte, 16> &bytes) noexcept : bytes(bytes) { }
	constexpr explicit UUID(const std::span<std::byte, 16> &bytes) noexcept {
		std::copy(bytes.begin(), bytes.end(), this->bytes.begin());
	}
	constexpr explicit UUID(const std::span<std::byte> &bytes) {
		if (bytes.size() != 16) {
			throw std::invalid_argument("wrong number of bytes");
		}
		std::copy(bytes.begin(), bytes.end(), this->bytes.begin());
	}

public:
	constexpr Variant _pure variant() const noexcept { unsigned v = fields.clock_seq_and_variant >> 13 | 0x3 << 3; v &= v >> 1; v &= v >> 1; return static_cast<Variant>(v); }
	constexpr Version _pure version() const noexcept { return static_cast<Version>(fields.time_hi_and_version >> 12); }
	constexpr uint64_t _pure time() const noexcept { return fields.time_low | uint64_t(fields.time_mid) << 32 | uint64_t(fields.time_hi_and_version & 0xFFF) << 48; }
	constexpr uint16_t _pure clock_seq() const noexcept { return fields.clock_seq_and_variant & 0x3FFF; }
	constexpr const std::array<std::byte, 6> & _const node() const noexcept { return fields.node; }

#ifdef __SIZEOF_INT128__
	constexpr explicit _pure operator bool () const noexcept { return wide; }
	constexpr bool _pure operator==(const UUID &other) const noexcept { return wide == other.wide; }
	constexpr auto _pure operator<=>(const UUID &other) const noexcept { return wide <=> other.wide; }
#else
	constexpr explicit _pure operator bool () const noexcept { return std::any_of(words.begin(), words.end(), std::identity { }); }
	constexpr bool _pure operator==(const UUID &other) const noexcept { return words == other.words; }
	constexpr auto _pure operator<=>(const UUID &other) const noexcept { return bytes <=> other.bytes; }
#endif

	friend std::ostream & operator<<(std::ostream &os, const UUID &uuid);

};

template <>
struct std::hash<UUID> {
	constexpr size_t _pure operator()(const UUID &uuid) const noexcept {
		size_t hash { };
		for (unsigned long word : uuid.words) {
			hash ^= word;
		}
		return hash;
	}
};
