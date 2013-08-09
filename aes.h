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
	AESBase() { }

};


template <size_t Key_Size>
class AESDecrypterBase;


template <size_t Key_Size>
class AESEncrypterBase : public AESBase<Key_Size> {
	friend AESDecrypterBase<Key_Size>;

protected:
	AESEncrypterBase() { }

public:
	size_t process(uint8_t (&out)[16], const uint8_t in[], size_t n);

};


template <size_t Key_Size>
class AESDecrypterBase : public AESBase<Key_Size> {

protected:
	AESDecrypterBase(const AESEncrypterBase<Key_Size> &encrypter);

public:
	size_t process(uint8_t (&out)[16], const uint8_t in[], size_t n);

};


class AES128Encrypter : public AESEncrypterBase<16> {

public:
	explicit AES128Encrypter(const uint8_t (&key)[16]);

};


class AES128Decrypter : public AESDecrypterBase<16> {

public:
	explicit AES128Decrypter(const uint8_t (&key)[16]);
	explicit AES128Decrypter(const AES128Encrypter &encrypter) : AESDecrypterBase<16>(encrypter) { }

};


class AES192Encrypter : public AESEncrypterBase<24> {

public:
	explicit AES192Encrypter(const uint8_t (&key)[24]);

};


class AES192Decrypter : public AESDecrypterBase<24> {

public:
	explicit AES192Decrypter(const uint8_t (&key)[24]);
	explicit AES192Decrypter(const AES192Encrypter &encrypter) : AESDecrypterBase<24>(encrypter) { }

};


class AES256Encrypter : public AESEncrypterBase<32> {

public:
	explicit AES256Encrypter(const uint8_t (&key)[32]);

};


class AES256Decrypter : public AESDecrypterBase<32> {

public:
	explicit AES256Decrypter(const uint8_t (&key)[32]);
	explicit AES256Decrypter(const AES256Encrypter &encrypter) : AESDecrypterBase<32>(encrypter) { }

};
