#include "io.h"

#include <cstddef>
#include <cstring>
#include <system_error>

#include "narrow.h"


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


ssize_t MemorySource::read(void *buf, size_t n) {
	if (n == 0) {
		return 0;
	}
	if (remaining == 0) {
		return -1;
	}
	size_t r = std::min(n, remaining);
	std::memcpy(buf, buffer, r);
	buffer = static_cast<const uint8_t *>(buffer) + r, remaining -= r;
	return r;
}


size_t MemorySink::write(const void *buf, size_t n, bool) {
	size_t r = std::min(n, remaining);
	if (r != 0) {
		std::memcpy(buffer, buf, r);
		buffer = static_cast<uint8_t *>(buffer) + r, remaining -= r;
	}
	return r;
}


ssize_t BufferedSourceBase::read(void *buf, size_t n, uint8_t buffer[], size_t buffer_size) {
	if (n == 0) {
		return 0;
	}
	ptrdiff_t r;
	if ((r = buffer_end - buffer_ptr) <= 0) {
		if ((r = source->read(buffer, buffer_size)) <= 0) {
			return r;
		}
		buffer_end = (buffer_ptr = buffer) + r;
	}
	if (n > static_cast<size_t>(r)) {
		n = r;
	}
	std::memcpy(buf, buffer_ptr, n);
	buffer_ptr += n;
	return n;
}


size_t BufferedSinkBase::write(const void *buf, size_t n, bool more, uint8_t buffer[], size_t buffer_size) {
	size_t ret = 0;
	do {
		if (n >= buffer_size && buffer_end == buffer) {
			return ret + sink->write(buf, n, more);
		}
		ptrdiff_t r;
		if ((r = buffer + buffer_size - buffer_end) > 0) {
			size_t w = std::min(n, static_cast<size_t>(r));
			std::memcpy(buffer_end, buf, w);
			buffer_end += w, ret += w;
			if ((r -= w) > 0 && more) {
				return ret;
			}
			buf = static_cast<const uint8_t *>(buf) + w, n -= w;
		}
		if ((r = buffer_end - buffer_ptr) > 0) {
			size_t w = sink->write(buffer_ptr, r, more || n > 0);
			if ((buffer_ptr += w) < buffer_end) {
				return ret;
			}
			buffer_end = buffer_ptr = buffer;
		}
	} while (n > 0);
	return ret;
}

bool BufferedSinkBase::finish(uint8_t buffer[], size_t buffer_size) {
	if (buffer_ptr < buffer_end) {
		this->write(nullptr, 0, false, buffer, buffer_size);
		if (buffer_ptr < buffer_end) {
			return false;
		}
	}
	return sink->finish();
}


ssize_t StringSource::read(void *buf, size_t n) {
	if (n == 0) {
		return 0;
	}
	ptrdiff_t r;
	if ((r = string->end() - string_itr) <= 0) {
		return -1;
	}
	if (n > static_cast<size_t>(r)) {
		n = r;
	}
	std::memcpy(buf, &*string_itr, n);
	string_itr += n;
	return n;
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
		sink->write_fully(pbase, r, more);
		this->setp(pbase, this->epptr());
	}
	else if (!more) {
		sink->write(nullptr, 0, false);
	}
	return 0;
}


std::streambuf * SourceSinkBuf::setbuf(char_type s[], std::streamsize n) {
	std::streamsize h = n / 2;
	this->SourceBuf::setbuf(s, h);
	this->SinkBuf::setbuf(s + h, n - h);
	return this;
}


MemoryBuf::MemoryBuf(const void *buf, size_t n) {
	this->pubsetbuf(const_cast<char *>(static_cast<const char *>(buf)), saturate<std::streamsize>(n));
}

MemoryBuf * MemoryBuf::setbuf(char s[], std::streamsize n) {
	this->setg(s, s, s + n);
	return this;
}
