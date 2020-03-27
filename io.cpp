#include "io.h"

#include <cstddef>
#include <system_error>

#include "narrow.h"


ssize_t Source::read(const BufferPointer bufs[], size_t count) {
	ssize_t ret = 0;
	for (size_t i = 0; i < count; ++i) {
		void *buf = bufs[i].ptr;
		size_t n = bufs[i].size;
		while (n > 0) {
			ssize_t r = this->read(buf, n);
			if (r <= 0) {
				return ret == 0 ? r : ret;
			}
			ret += r;
			buf = static_cast<uint8_t *>(buf) + r, n -= r;
		}
	}
	return ret;
}

void Source::read_fully(void *buf, size_t n) {
	while (n > 0) {
		ssize_t r = this->read(buf, n);
		if (_likely(r > 0)) {
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

void Source::read_fully(const BufferPointer bufs[], size_t count) {
	while (count > 0) {
		ssize_t r = this->read(bufs, count);
		if (_likely(r > 0)) {
			while ((r -= bufs[0].size) > 0) {
				++bufs, --count;
			}
			if (r < 0) {
				this->read_fully(static_cast<uint8_t *>(bufs[0].ptr) + bufs[0].size + r, -r);
			}
			++bufs, --count;
		}
		else if (r < 0) {
			throw std::ios_base::failure("premature EOF");
		}
		else {
			throw std::logic_error("non-blocking read in blocking context");
		}
	}
}


size_t Sink::write(const BufferPointer bufs[], size_t count) {
	size_t ret = 0;
	for (size_t i = 0; i < count; ++i) {
		const void *buf = bufs[i].ptr;
		size_t n = bufs[i].size;
		while (n > 0) {
			size_t w = this->write(buf, n);
			if (w == 0) {
				return ret;
			}
			ret += w;
			buf = static_cast<const uint8_t *>(buf) + w, n -= w;
		}
	}
	return ret;
}

void Sink::write_fully(const void *buf, size_t n) {
	while (n > 0) {
		size_t w = this->write(buf, n);
		if (_likely(w > 0)) {
			buf = static_cast<const uint8_t *>(buf) + w, n -= w;
		}
		else {
			throw std::logic_error("non-blocking write in blocking context");
		}
	}
}

void Sink::write_fully(const BufferPointer bufs[], size_t count) {
	while (count > 0) {
		size_t w = this->write(bufs, count);
		if (_likely(w > 0)) {
			while (w > bufs[0].size) {
				w -= bufs[0].size;
				++bufs, --count;
			}
			if (w < bufs[0].size) {
				this->write_fully(static_cast<const uint8_t *>(bufs[0].ptr) + w, bufs[0].size - w);
			}
			++bufs, --count;
		}
		else {
			throw std::logic_error("non-blocking write in blocking context");
		}
	}
}

void Sink::flush_fully() {
	if (_unlikely(!this->flush())) {
		throw std::logic_error("non-blocking write in blocking context");
	}
}


ssize_t LimitedSource::read(void *buf, size_t n) {
	if (n == 0) {
		return 0;
	}
	if (n > remaining) {
		if (remaining == 0) {
			return -1;
		}
		n = remaining;
	}
	ssize_t r = source.read(buf, n);
	if (r > 0) {
		remaining -= r;
	}
	return r;
}


size_t LimitedSink::write(const void *buf, size_t n) {
	if (n > remaining) {
		n = remaining;
	}
	if (n == 0) {
		return 0;
	}
	size_t w = sink.write(buf, n);
	remaining -= w;
	return w;
}


ssize_t MemorySource::read(void *buf, size_t n) {
	size_t grem = this->grem();
	if (n > grem) {
		if (grem == 0) {
			return -1;
		}
		n = grem;
	}
	std::memcpy(buf, gptr, n), gptr += n;
	return n;
}


size_t MemorySink::write(const void *buf, size_t n) {
	size_t prem = this->prem();
	if (n > prem) {
		n = prem;
	}
	std::memcpy(pptr, buf, n), pptr += n;
	return n;
}


ssize_t BufferSource::read(void *buf, size_t n) {
	size_t grem = buffer.grem();
	if (n > grem) {
		if (grem == 0) {
			return -1;
		}
		n = grem;
	}
	std::memcpy(buf, buffer.gptr, n), buffer.gptr += n;
	return n;
}


size_t StaticBufferSink::write(const void *buf, size_t n) {
	size_t prem = buffer.prem();
	if (n > prem) {
		n = prem;
	}
	std::memcpy(buffer.pptr, buf, n), buffer.pptr += n;
	return n;
}


size_t BufferSink::write(const void *buf, size_t n) {
	this->append(buf, n);
	return n;
}


size_t StringSink::write(const void *buf, size_t n) {
	string.append(static_cast<const char *>(buf), n);
	return n;
}


ssize_t BufferedSourceBase::read(void *buf, size_t n) {
	if (n == 0) {
		return 0;
	}
	size_t b = buf_pptr - buf_gptr;
	if (b > 0) {
		if (n <= b) {
			std::memcpy(buf, buf_gptr, n), buf_gptr += n;
			return n;
		}
		std::memcpy(buf, buf_gptr, b), buf_gptr += b;
		buf = static_cast<uint8_t *>(buf) + b, n -= b;
	}
	ssize_t r = buf_eptr - buf_bptr;
	if (n >= static_cast<size_t>(r)) {
		r = source.read(buf, n);
		return r >= 0 ? b + r : b == 0 ? r : b;
	}
	if ((r = source.read(buf_gptr = buf_bptr, r)) <= 0) {
		buf_pptr = buf_bptr;
		return b == 0 ? r : b;
	}
	buf_pptr = buf_bptr + r;
	if (n <= static_cast<size_t>(r)) {
		std::memcpy(buf, buf_gptr, n), buf_gptr += n;
		return b + n;
	}
	std::memcpy(buf, buf_gptr, r), buf_gptr += r;
	return b + r;
}


size_t BufferedSinkBase::write(const void *buf, size_t n) {
	if (n == 0) {
		return 0;
	}
	size_t r = buf_eptr - buf_pptr;
	if (r > 0) {
		if (n < r) {
			std::memcpy(buf_pptr, buf, n), buf_pptr += n;
			return n;
		}
		std::memcpy(buf_pptr, buf, r), buf_pptr += r;
		buf = static_cast<const uint8_t *>(buf) + r, n -= r;
	}
	size_t b = buf_eptr - buf_gptr;
	if (b > 0 && (buf_gptr += sink.write(buf_gptr, b)) < buf_eptr) {
		return r;
	}
	if (n >= static_cast<size_t>(buf_eptr - buf_bptr)) {
		return r + sink.write(buf, n);
	}
	std::memcpy(buf_gptr = buf_bptr, buf, n), buf_pptr = buf_bptr + n;
	return r + n;
}

bool BufferedSinkBase::flush() {
	size_t b = buf_pptr - buf_gptr;
	return (b == 0 || (buf_gptr += sink.write(buf_gptr, b)) == buf_pptr) && sink.flush();
}


ssize_t DelimitedSource::read(void *buf, size_t n) {
	ssize_t r = 0;
	while (n > 0) {
		std::ptrdiff_t d;
		if ((d = delimiter.end() - delim_itr) <= 0) {
			return r == 0 ? -1 : r;
		}
		ssize_t s;
		if ((s = source.read(buf, std::min(static_cast<size_t>(d), n))) <= 0) {
			return r == 0 ? s : r;
		}
		for (ssize_t i = 0; i < s; ++i) {
			char c = static_cast<char *>(buf)[i];
			delim_itr = c == *delim_itr ? delim_itr + 1 : c == *delimiter.begin() ? delimiter.begin() + 1 : delimiter.begin();
		}
		buf = static_cast<char *>(buf) + s, n -= s, r += s;
	}
	return r;
}


ssize_t Tap::read(void *buf, size_t n) {
	ssize_t r;
	if ((r = source.read(buf, n)) > 0) {
		sink.write_fully(buf, r);
	}
	return r;
}


ssize_t StreamBufSourceSink::read(void *buf, size_t n) {
	auto r = sb.sgetn(static_cast<std::streambuf::char_type *>(buf), saturate<std::streamsize>(n));
	return r == 0 ? -1 : r;
}

size_t StreamBufSourceSink::avail() {
	return saturate<size_t>(sb.in_avail());
}

size_t StreamBufSourceSink::write(const void *buf, size_t n) {
	return static_cast<size_t>(sb.sputn(static_cast<const std::streambuf::char_type *>(buf), saturate<std::streamsize>(n)));
}


SourceBuf::SourceBuf(Source &source) noexcept : source(source) {
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
	return source.avail();
}

std::streambuf::int_type SourceBuf::underflow() {
	char_type *gptr = this->gptr();
	if (gptr == this->egptr()) {
		char_type *eback = this->eback();
		ssize_t n = source.read(eback, this->pbase() - eback);
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
		r += std::max(source.read(s, n), ssize_t());
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
	return pptr > this->pbase() && this->sync(true) ? 0 : sink.write(s, n);
}

int SinkBuf::sync(bool more) {
	char_type *pbase = this->pbase();
	ptrdiff_t r = this->pptr() - pbase;
	if (r > 0) {
		sink.write_fully(pbase, r);
		this->setp(pbase, this->epptr());
	}
	if (!more) {
		sink.flush_fully();
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
