#pragma once

#include "compiler.h"
#include "io.h"


template <typename Codec, size_t BufferSize = 512>
class CodecSource : public Source {

public:
	typedef Codec codec_type;

public:
	static constexpr size_t buffer_size = BufferSize;

private:
	Source *source;
	codec_type codec;
	uint8_t ibuf[buffer_size];
	uint8_t *ibuf_ptr, *ibuf_eptr;

public:
	template <typename... Args>
	explicit CodecSource(Source *source, Args&&... args) : source(source), codec(std::forward<Args>(args)...), ibuf_ptr(ibuf), ibuf_eptr(ibuf) { }

public:
	ssize_t read(void *buf, size_t n) override;

};


template <typename Codec, size_t BufferSize = 512>
class CodecSink : public Sink {

public:
	typedef Codec codec_type;

public:
	static constexpr size_t buffer_size = BufferSize;

private:
	Sink *sink;
	codec_type codec;
	uint8_t obuf[buffer_size];
	uint8_t *obuf_ptr, *obuf_eptr;

public:
	template <typename... Args>
	explicit CodecSink(Sink *sink, Args&&... args) : sink(sink), codec(std::forward<Args>(args)...), obuf_ptr(obuf), obuf_eptr(obuf) { }

public:
	size_t write(const void *buf, size_t n, bool more = false) override;
	bool finish() override;

};


template <typename Codec, typename... Args>
size_t transcode(void *out, size_t n_out, const void *in, size_t n_in, Args&&... args) {
	Codec codec(std::forward<Args>(args)...);
	uint8_t *obuf_ptr = static_cast<uint8_t *>(out), *obuf_eptr = obuf_ptr + n_out;
	const uint8_t *ibuf_ptr = static_cast<const uint8_t *>(in);
	if (!codec.process(obuf_ptr, n_out, ibuf_ptr, n_in) || !codec.finish(obuf_ptr, obuf_eptr - obuf_ptr)) {
		return SIZE_MAX;
	}
	return obuf_ptr - static_cast<uint8_t *>(out);
}

template <typename Codec, typename... Args>
static inline size_t transcode(void *out, size_t n_out, const std::string &in, Args&&... args) {
	return ::transcode<Codec, Args...>(out, n_out, in.data(), in.size(), std::forward<Args>(args)...);
}

template <typename Codec, typename... Args>
void transcode(std::string &out, const void *in, size_t n_in, Args&&... args) {
	StringSink ss(&out);
	CodecSink<Codec> cs(&ss, std::forward<Args>(args)...);
	cs.write_fully(in, n_in);
	cs.finish();
}

template <typename Codec, typename... Args>
static inline void transcode(std::string &out, const std::string &in, Args&&... args) {
	::transcode<Codec, Args...>(out, in.data(), in.size(), std::forward<Args>(args)...);
}

template <typename Codec>
static inline std::string transcode(const void *in, size_t n_in) {
	std::string ret;
	::transcode<Codec>(ret, in, n_in);
	return ret;
}

template <typename Codec>
static inline std::string transcode(const std::string &in) {
	std::string ret;
	::transcode<Codec>(ret, in.data(), in.size());
	return ret;
}
