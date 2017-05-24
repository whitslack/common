#include "gzip.h"

#include <cstring>

#include "narrow.h"

using z_avail_t = decltype(z_stream::avail_in);
static_assert(std::is_same<decltype(z_stream::avail_out), z_avail_t>::value, "");


GZipSource::GZipSource(Source &source) : source(source) {
	std::memset(&stream, 0, sizeof stream);
	stream.next_in = ibuf;
	if (::inflateInit2(&stream, 16 /* gzip format only */ + 15 /* window bits */) != Z_OK) {
		throw std::runtime_error(stream.msg);
	}
}

GZipSource::~GZipSource() {
	if (::inflateEnd(&stream) != Z_OK) {
		throw std::runtime_error(stream.msg);
	}
}

ssize_t GZipSource::read(void *buf, size_t n) {
	uint8_t *iend = const_cast<uint8_t *>(stream.next_in + stream.avail_in);
	std::ptrdiff_t d;
	if ((d = ibuf + sizeof ibuf - iend) > 0) {
		ssize_t r = source.read(iend, d);
		if (r > 0) {
			stream.avail_in += static_cast<z_avail_t>(r);
		}
	}
	stream.next_out = static_cast<uint8_t *>(buf);
	n = stream.avail_out = saturate<z_avail_t>(n);
	int error;
	if ((error = ::inflate(&stream, Z_NO_FLUSH)) != Z_OK && error != Z_STREAM_END) {
		throw std::ios_base::failure(stream.msg);
	}
	if (stream.avail_in == 0) {
		stream.next_in = ibuf;
	}
	ssize_t r = n - stream.avail_out;
	return r == 0 ? error == Z_STREAM_END ? -1 : 0 : r;
}


GZipSink::GZipSink(Sink &sink, int level) : sink(sink) {
	std::memset(&stream, 0, sizeof stream);
	stream.next_out = obuf;
	if (::deflateInit2(&stream, level, Z_DEFLATED, 16 /* gzip format */ + 15 /* window bits */, 9, Z_DEFAULT_STRATEGY) != Z_OK) {
		throw std::runtime_error(stream.msg);
	}
}

GZipSink::~GZipSink() {
	if (::deflateEnd(&stream) != Z_OK) {
		throw std::runtime_error(stream.msg);
	}
}

size_t GZipSink::write(const void *buf, size_t n) {
	this->write(buf, n, Z_NO_FLUSH);
	return n;
}

bool GZipSink::flush() {
	size_t n = 0;
	return this->write(nullptr, n, Z_FINISH) == Z_STREAM_END && stream.avail_out == 0 && sink.flush();
}

int GZipSink::write(const void *buf, size_t &n, int flush) {
	uint8_t *obegin = stream.next_out;
	stream.next_out += stream.avail_out;
	stream.avail_out = static_cast<z_avail_t>(obuf + sizeof obuf - stream.next_out);
	stream.next_in = static_cast<const uint8_t *>(buf);
	n = stream.avail_in = saturate<z_avail_t>(n);
	int error;
	if ((error = ::deflate(&stream, flush)) != Z_OK && error != Z_STREAM_END) {
		throw std::ios_base::failure(stream.msg);
	}
	n -= stream.avail_in;
	stream.avail_out = static_cast<z_avail_t>(stream.next_out - obegin);
	stream.next_out = obegin;
	if (stream.avail_out > 0) {
		size_t w = sink.write(stream.next_out, stream.avail_out);
		if ((stream.avail_out -= static_cast<z_avail_t>(w)) == 0) {
			stream.next_out = obuf;
		}
		else {
			stream.next_out += w;
		}
	}
	return error;
}
