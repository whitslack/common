#pragma once

#include "compiler.h"
#include "io.h"


template <typename Codec>
class CodecSource : public Source {

public:
	typedef Codec codec_type;

private:
	Source *source;
	codec_type codec;
	uint8_t ibuf[codec_type::input_block_size], obuf[codec_type::output_block_size];
	size_t ipos, opos, osize;

public:
	template <typename... Args>
	explicit CodecSource(Source *source, Args&&... args) : source(source), codec(std::forward<Args>(args)...), ipos(), opos(), osize() { }

public:
	ssize_t read(void *buf, size_t n) override;
	size_t avail() override _pure;

};


template <typename Codec>
class CodecSink : public Sink {

public:
	typedef Codec codec_type;

private:
	Sink *sink;
	codec_type codec;
	uint8_t ibuf[codec_type::input_block_size], obuf[codec_type::output_block_size];
	size_t ipos, opos, osize;

public:
	template <typename... Args>
	explicit CodecSink(Sink *sink, Args&&... args) : sink(sink), codec(std::forward<Args>(args)...), ipos(), opos(), osize() { }

public:
	size_t write(const void *buf, size_t n, bool more = false) override;
	bool finish() override;

};


template <typename Codec, typename... Args>
size_t transcode(void *out, size_t n_out, const void *in, size_t n_in, Args&&... args) {
	MemorySink ms(out, n_out);
	CodecSink<Codec> cs(&ms, std::forward<Args>(args)...);
	cs.write_fully(in, n_in);
	cs.finish();
	return n_out - ms.remaining;
}

template <typename Codec, typename... Args>
static inline size_t transcode(void *out, size_t n_out, const std::string &in, Args&&... args) {
	return ::transcode<Codec, Args...>(out, n_out, in.data(), in.size(), std::forward<Args>(args)...);
}

template <typename Codec, typename... Args>
void transcode(std::string &out, const void *in, size_t n_in, Args&&... args) {
	out.resize((n_in + Codec::input_block_size - 1) / Codec::input_block_size * Codec::output_block_size);
	out.resize(::transcode<Codec, Args...>(&out.front(), out.size(), in, n_in, std::forward<Args>(args)...));
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
