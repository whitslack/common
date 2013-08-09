#include <streambuf>

template <typename CharT, typename Traits = std::char_traits<CharT>>
class basic_rawstrbuf : public std::basic_streambuf<CharT, Traits> {

public:
	basic_rawstrbuf() { }

	basic_rawstrbuf(const CharT s[], std::streamsize n) {
		this->pubsetbuf(const_cast<CharT *>(s), n);
	}

protected:
	std::basic_streambuf<CharT, Traits> * setbuf(CharT s[], std::streamsize n) override {
		this->setg(s, s, s + n);
		return this;
	}

};

typedef basic_rawstrbuf<char> rawstrbuf;
