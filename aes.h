#include <cstddef>
#include <cstdint>


template <size_t Key_Size>
class AESBase {

public:
	static constexpr size_t input_block_size = 16, output_block_size = 16;
	static constexpr size_t key_size = Key_Size, sched_size = key_size + 28;

protected:
	uint32_t key_schedule[sched_size];

protected:
	AESBase() = default;

};


template <size_t Key_Size>
class AESDecrypterBase;


template <size_t Key_Size>
class AESEncrypterBase : public AESBase<Key_Size> {
	friend AESDecrypterBase<Key_Size>;

protected:
	AESEncrypterBase() = default;

public:
	size_t process(std::byte (&out)[16], const std::byte in[], size_t n) noexcept;

};


template <size_t Key_Size>
class AESDecrypterBase : public AESBase<Key_Size> {

protected:
	explicit AESDecrypterBase(const AESEncrypterBase<Key_Size> &encrypter) noexcept;

public:
	size_t process(std::byte (&out)[16], const std::byte in[], size_t n) noexcept;

};


class AES128Encrypter : public AESEncrypterBase<16> {

public:
	explicit AES128Encrypter(const std::byte (&key)[16]) noexcept;

};


class AES128Decrypter : public AESDecrypterBase<16> {

public:
	explicit AES128Decrypter(const std::byte (&key)[16]) noexcept : AESDecrypterBase<16>(AES128Encrypter(key)) { }
	explicit AES128Decrypter(const AES128Encrypter &encrypter) noexcept : AESDecrypterBase<16>(encrypter) { }

};


class AES192Encrypter : public AESEncrypterBase<24> {

public:
	explicit AES192Encrypter(const std::byte (&key)[24]) noexcept;

};


class AES192Decrypter : public AESDecrypterBase<24> {

public:
	explicit AES192Decrypter(const std::byte (&key)[24]) noexcept : AESDecrypterBase<24>(AES192Encrypter(key)) { }
	explicit AES192Decrypter(const AES192Encrypter &encrypter) noexcept : AESDecrypterBase<24>(encrypter) { }

};


class AES256Encrypter : public AESEncrypterBase<32> {

public:
	explicit AES256Encrypter(const std::byte (&key)[32]) noexcept;

};


class AES256Decrypter : public AESDecrypterBase<32> {

public:
	explicit AES256Decrypter(const std::byte (&key)[32]) noexcept : AESDecrypterBase<32>(AES256Encrypter(key)) { }
	explicit AES256Decrypter(const AES256Encrypter &encrypter) noexcept : AESDecrypterBase<32>(encrypter) { }

};
