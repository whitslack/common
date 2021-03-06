#include "io.tcc"

#include "narrow.h"


template class Readable<Source>;
template class Writable<Sink>;
template class Flushable<Sink>;

template class InputSource<std::reference_wrapper<Source>>;
template class OutputSink<std::reference_wrapper<Sink>>;
template class InputSource<LimitedReadable<>>;
template class OutputSink<LimitedWritable<>>;
template class InputSource<DelimitedReadable<>>;


ssize_t read(void *dst, ConstBufferView &src, size_t n) noexcept {
	size_t grem = src.grem();
	if (n > grem) {
		if (grem == 0) {
			return -1;
		}
		n = grem;
	}
	std::memcpy(dst, src.gptr, n), src.gptr += n;
	return n;
}

size_t write(BufferView &dst, const void *src, size_t n) noexcept {
	size_t prem = dst.prem();
	if (n > prem) {
		n = prem;
	}
	std::memcpy(dst.pptr, src, n), dst.pptr += n;
	return n;
}


ssize_t BufferedSource::read(void *buf, size_t n) {
	if (n == 0) {
		return 0;
	}
	size_t b = pptr - gptr;
	if (b > 0) {
		if (n <= b) {
			std::memcpy(buf, gptr, n), gptr += n;
			return n;
		}
		std::memcpy(buf, gptr, b), gptr += b;
		buf = static_cast<std::byte *>(buf) + b, n -= b;
	}
	ssize_t r = eptr - bptr;
	if (n >= static_cast<size_t>(r)) {
		r = source.read(buf, n);
		return r >= 0 ? b + r : b == 0 ? r : b;
	}
	if ((r = source.read(gptr = bptr, r)) <= 0) {
		pptr = bptr;
		return b == 0 ? r : b;
	}
	pptr = bptr + r;
	if (n <= static_cast<size_t>(r)) {
		std::memcpy(buf, gptr, n), gptr += n;
		return b + n;
	}
	std::memcpy(buf, gptr, r), gptr += r;
	return b + r;
}


size_t BufferedSink::write(const void *buf, size_t n) {
	if (n == 0) {
		return 0;
	}
	size_t r = eptr - pptr;
	if (r > 0) {
		if (n < r) {
			std::memcpy(pptr, buf, n), pptr += n;
			return n;
		}
		std::memcpy(pptr, buf, r), pptr += r;
		buf = static_cast<const std::byte *>(buf) + r, n -= r;
	}
	size_t b = eptr - gptr;
	if (b > 0 && (gptr += sink.write(gptr, b)) < eptr) {
		return r;
	}
	if (n >= static_cast<size_t>(eptr - bptr)) {
		return r + sink.write(buf, n);
	}
	std::memcpy(gptr = bptr, buf, n), pptr = bptr + n;
	return r + n;
}

bool BufferedSink::flush() {
	size_t b = pptr - gptr;
	return (b == 0 || (gptr += sink.write(gptr, b)) == pptr) && sink.flush();
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
