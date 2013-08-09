#include "io.h"

#include <cstddef>
#include <cstring>
#include <system_error>


void Source::read_fully(void *buf, size_t n) {
	while (n > 0) {
		ssize_t r = this->read(buf, n);
		if (r > 0) {
			buf = static_cast<uint8_t *>(buf) + r, n -= r;
		}
		else if (r < 0) {
			throw std::ios_base::failure("premature EOF");
		}
		else {
			throw std::logic_error("non-blocking read in blocking context");
		}
	}
}


void Sink::write_fully(const void *buf, size_t n, bool more) {
	while (n > 0) {
		size_t w = this->write(buf, n, more);
		if (w > 0) {
			buf = static_cast<const uint8_t *>(buf) + w, n -= w;
		}
		else {
			throw std::logic_error("non-blocking write in blocking context");
		}
	}
}


ssize_t StringSource::read(void *buf, size_t n) {
	if (n == 0) {
		return 0;
	}
	size_t r = string->end() - string_itr;
	if (r == 0) {
		return -1;
	}
	if (r > n) {
		r = n;
	}
	std::memcpy(buf, &*string_itr, r);
	string_itr += r;
	return r;
}

size_t StringSource::avail() {
	return string->end() - string_itr;
}


size_t StringSink::write(const void *buf, size_t n, bool) {
	string->append(static_cast<const char *>(buf), n);
	return n;
}


SourceBuf::SourceBuf(Source *source) : source(source) {
	this->setbuf(&gbuf, 1);
}

std::streambuf * SourceBuf::setbuf(char_type s[], std::streamsize n) {
	if (n <= 0) {
		s = &gbuf, n = 1;
	}
	this->setg(s, s + n, s + n);
	this->setp(s + n, s + n);
	return this;
}

std::streamsize SourceBuf::showmanyc() {
	return source->avail();
}

std::streambuf::int_type SourceBuf::underflow() {
	char_type *gptr = this->gptr();
	if (gptr == this->egptr()) {
		char_type *eback = this->eback();
		ssize_t n = source->read(eback, this->pbase() - eback);
		if (n <= 0) {
			this->setg(eback, eback, eback);
			return traits_type::eof();
		}
		this->setg(eback, gptr = eback, eback + n);
	}
	return traits_type::to_int_type(*gptr);
}

std::streamsize SourceBuf::xsgetn(char_type s[], std::streamsize n) {
	char_type *gptr = this->gptr();
	ptrdiff_t r = std::min(n, this->egptr() - gptr);
	if (r > 0) {
		std::memcpy(s, gptr, r);
		this->gbump(static_cast<int>(r));
		s += r, n -= r;
	}
	if (n > 0) {
		r += std::max(source->read(s, n), ssize_t());
	}
	return r;
}


std::streambuf * SinkBuf::setbuf(char_type s[], std::streamsize n) {
	if (n <= 0) {
		s = nullptr, n = 0;
	}
	this->setp(s, s + n);
	return this;
}

int SinkBuf::sync() {
	return this->sync(false);
}

std::streambuf::int_type SinkBuf::overflow(int_type ch) {
	if (this->sync(true)) {
		return traits_type::eof();
	}
	if (!traits_type::eq_int_type(ch, traits_type::eof())) {
		char_type c = traits_type::to_char_type(ch);
		char_type *pptr = this->pptr();
		if (pptr < this->epptr()) {
			*pptr = c;
			this->pbump(1);
		}
		else if (this->xsputn(&c, 1) == 0) {
			return traits_type::eof();
		}
	}
	return traits_type::not_eof(0);
}

std::streamsize SinkBuf::xsputn(const char_type s[], std::streamsize n) {
	char_type *pptr = this->pptr(), *epptr = this->epptr();
	if (n <= epptr - pptr) {
		std::memcpy(pptr, s, n);
		this->pbump(static_cast<int>(n));
		return n;
	}
	return pptr > this->pbase() && this->sync(true) ? 0 : sink->write(s, n, true);
}

int SinkBuf::sync(bool more) {
	char_type *pbase = this->pbase();
	ptrdiff_t r = this->pptr() - pbase;
	if (r > 0) {
		ssize_t n = sink->write(pbase, r, more);
		if (n > 0) {
			this->pbump(static_cast<int>(-n));
			if ((r -= n) > 0) {
				std::memcpy(pbase, pbase + n, r);
			}
		}
	}
	return r == 0 ? 0 : -1;
}


std::streambuf * SourceSinkBuf::setbuf(char_type s[], std::streamsize n) {
	std::streamsize h = n / 2;
	this->SourceBuf::setbuf(s, h);
	this->SinkBuf::setbuf(s + h, n - h);
	return this;
}
