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
