#include "gzip.h"

#include <cstring>

#include "narrow.h"

using z_avail_t = decltype(z_stream::avail_in);
static_assert(std::is_same_v<decltype(z_stream::avail_out), z_avail_t>, "");


GZipSource::GZipSource(Source &source) : source(source) {
	std::memset(&stream, 0, sizeof stream);
	stream.next_in = reinterpret_cast<const Bytef *>(ibuf);
	if (_unlikely(::inflateInit2(&stream, 16 /* gzip format only */ + 15 /* window bits */) != Z_OK)) {
		throw std::runtime_error(stream.msg);
	}
}

GZipSource::~GZipSource() {
	if (_unlikely(::inflateEnd(&stream) != Z_OK)) {
		throw std::runtime_error(stream.msg);
	}
}

ssize_t GZipSource::read(void *buf, size_t n) {
	std::byte *iend = const_cast<std::byte *>(reinterpret_cast<const std::byte *>(stream.next_in) + stream.avail_in);
	std::ptrdiff_t d;
	if ((d = ibuf + sizeof ibuf - iend) > 0) {
		ssize_t r = source.read(iend, d);
		if (r > 0) {
			stream.avail_in += static_cast<z_avail_t>(r);
		}
	}
	stream.next_out = static_cast<Bytef *>(buf);
	n = stream.avail_out = saturate<z_avail_t>(n);
	int error = ::inflate(&stream, Z_NO_FLUSH);
	if (_unlikely(error != Z_OK && error != Z_STREAM_END)) {
		throw std::ios_base::failure(stream.msg);
	}
	if (stream.avail_in == 0) {
		stream.next_in = reinterpret_cast<const Bytef *>(ibuf);
	}
	ssize_t r = n - stream.avail_out;
	return r == 0 ? error == Z_STREAM_END ? -1 : 0 : r;
}


GZipSink::GZipSink(Sink &sink, int level) : sink(sink) {
	std::memset(&stream, 0, sizeof stream);
	stream.next_out = reinterpret_cast<Bytef *>(obuf);
	if (_unlikely(::deflateInit2(&stream, level, Z_DEFLATED, 16 /* gzip format */ + 15 /* window bits */, 9, Z_DEFAULT_STRATEGY) != Z_OK)) {
		throw std::runtime_error(stream.msg);
	}
}

GZipSink::~GZipSink() {
	if (_unlikely(::deflateEnd(&stream) != Z_OK)) {
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
	std::byte *obegin = reinterpret_cast<std::byte *>(stream.next_out);
	stream.next_out += stream.avail_out;
	stream.avail_out = static_cast<z_avail_t>(obuf + sizeof obuf - reinterpret_cast<std::byte *>(stream.next_out));
	stream.next_in = static_cast<const Bytef *>(buf);
	n = stream.avail_in = saturate<z_avail_t>(n);
	int error = ::deflate(&stream, flush);
	if (_unlikely(error != Z_OK && error != Z_STREAM_END)) {
		throw std::ios_base::failure(stream.msg);
	}
	n -= stream.avail_in;
	stream.avail_out = static_cast<z_avail_t>(reinterpret_cast<std::byte *>(stream.next_out) - obegin);
	stream.next_out = reinterpret_cast<Bytef *>(obegin);
	if (stream.avail_out > 0) {
		size_t w = sink.write(stream.next_out, stream.avail_out);
		if ((stream.avail_out -= static_cast<z_avail_t>(w)) == 0) {
			stream.next_out = reinterpret_cast<Bytef *>(obuf);
		}
		else {
			stream.next_out += w;
		}
	}
	return error;
}
