#pragma once

#include <string_view>

#include "compiler.h"
#include "io.h"


template <typename Codec, size_t BufferSize = 512>
class CodecSource : public Source {

public:
	typedef Codec codec_type;

public:
	static constexpr size_t buffer_size = BufferSize;

private:
	Source &source;
	codec_type codec;
	uint8_t ibuf[buffer_size];
	uint8_t *ibuf_ptr, *ibuf_eptr;

public:
	template <typename... Args>
	explicit CodecSource(Source &source, Args&&... args) : source(source), codec(std::forward<Args>(args)...), ibuf_ptr(ibuf), ibuf_eptr(ibuf) { }

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
	Sink &sink;
	codec_type codec;
	uint8_t obuf[buffer_size];
	uint8_t *obuf_ptr, *obuf_eptr;

public:
	template <typename... Args>
	explicit CodecSink(Sink &sink, Args&&... args) : sink(sink), codec(std::forward<Args>(args)...), obuf_ptr(obuf), obuf_eptr(obuf) { }

public:
	size_t write(const void *buf, size_t n) override;
	bool flush() override;

};


template <typename Codec, size_t OBufSize = 512, size_t IBufSize = 512, typename... Args>
Sink & transcode(Sink &sink, Source &source, Args&&... args) {
	Codec codec(std::forward<Args>(args)...);
	uint8_t obuf[OBufSize], ibuf[IBufSize];
	for (;;) {
		bool finished;
		ssize_t r = source.read(ibuf, sizeof ibuf);
		if (r < 0) {
			do {
				uint8_t *obuf_ptr = obuf;
				finished = codec.finish(obuf_ptr, sizeof obuf);
				sink.write_fully(obuf, obuf_ptr - obuf);
			} while (!finished);
			return sink;
		}
		const uint8_t *ibuf_ptr = ibuf, *ibuf_eptr = ibuf + r;
		do {
			uint8_t *obuf_ptr = obuf;
			finished = codec.process(obuf_ptr, sizeof obuf, ibuf_ptr, ibuf_eptr - ibuf_ptr);
			sink.write_fully(obuf, obuf_ptr - obuf);
		} while (!finished);
	}
}

template <typename Codec, size_t OBufSize = 512, typename... Args>
Sink & transcode(Sink &sink, const void *in, size_t n_in, Args&&... args) {
	Codec codec(std::forward<Args>(args)...);
	uint8_t obuf[OBufSize];
	auto ibuf_ptr = static_cast<const uint8_t *>(in), ibuf_eptr = ibuf_ptr + n_in;
	bool finished;
	do {
		uint8_t *obuf_ptr = obuf;
		finished = codec.process(obuf_ptr, sizeof obuf, ibuf_ptr, ibuf_eptr - ibuf_ptr);
		sink.write_fully(obuf, obuf_ptr - obuf);
	} while (!finished);
	do {
		uint8_t *obuf_ptr = obuf;
		finished = codec.finish(obuf_ptr, sizeof obuf);
		sink.write_fully(obuf, obuf_ptr - obuf);
	} while (!finished);
	return sink;
}

template <typename Codec, size_t OBufSize = 512, typename... Args>
static inline Sink & transcode(Sink &sink, std::string_view in, Args&&... args) {
	return ::transcode<Codec, Args...>(sink, in.data(), in.size(), std::forward<Args>(args)...);
}

template <typename Codec, size_t IBufSize = 512, typename... Args>
size_t transcode(void * _restrict out, size_t n_out, Source &source, Args&&... args) {
	Codec codec(std::forward<Args>(args)...);
	auto obuf_ptr = static_cast<uint8_t *>(out), obuf_eptr = obuf_ptr + n_out;
	uint8_t ibuf[IBufSize];
	for (;;) {
		bool finished;
		ssize_t r = source.read(ibuf, sizeof ibuf);
		if (r < 0) {
			return codec.finish(obuf_ptr, obuf_eptr - obuf_ptr) ? obuf_ptr - static_cast<uint8_t *>(out) : SIZE_MAX;
		}
		const uint8_t *ibuf_ptr = ibuf;
		if (!codec.process(obuf_ptr, obuf_eptr - obuf_ptr, ibuf_ptr, r)) {
			return SIZE_MAX;
		}
	}
}

template <typename Codec, size_t OBufSize = 512, size_t IBufSize = 512, typename... Args>
std::string & transcode(std::string &out, Source &source, Args&&... args) {
	Codec codec(std::forward<Args>(args)...);
	uint8_t ibuf[IBufSize];
	for (;;) {
		bool finished;
		ssize_t r = source.read(ibuf, sizeof ibuf);
		if (r < 0) {
			do {
				auto size = out.size();
				out.resize(size + OBufSize);
				auto obuf_ptr = reinterpret_cast<uint8_t *>(&out.front() + size);
				finished = codec.finish(obuf_ptr, OBufSize);
				out.resize(obuf_ptr - reinterpret_cast<uint8_t *>(&out.front()));
			} while (!finished);
			return out;
		}
		const uint8_t *ibuf_ptr = ibuf, *ibuf_eptr = ibuf + r;
		do {
			auto size = out.size();
			out.resize(size + OBufSize);
			auto obuf_ptr = reinterpret_cast<uint8_t *>(&out.front() + size);
			finished = codec.process(obuf_ptr, OBufSize, ibuf_ptr, ibuf_eptr - ibuf_ptr);
			out.resize(obuf_ptr - reinterpret_cast<uint8_t *>(&out.front()));
		} while (!finished);
	}
}

template <typename Codec, typename... Args>
size_t transcode(void * _restrict out, size_t n_out, const void *in, size_t n_in, Args&&... args) {
	Codec codec(std::forward<Args>(args)...);
	auto obuf_ptr = static_cast<uint8_t *>(out), obuf_eptr = obuf_ptr + n_out;
	auto ibuf_ptr = static_cast<const uint8_t *>(in);
	return codec.process(obuf_ptr, n_out, ibuf_ptr, n_in) && codec.finish(obuf_ptr, obuf_eptr - obuf_ptr) ? obuf_ptr - static_cast<uint8_t *>(out) : SIZE_MAX;
}

template <typename Codec, typename... Args>
static inline size_t transcode(void * _restrict out, size_t n_out, std::string_view in, Args&&... args) {
	return ::transcode<Codec, Args...>(out, n_out, in.data(), in.size(), std::forward<Args>(args)...);
}

template <typename Codec, size_t OBufSize = 512, typename... Args>
std::string & transcode(std::string &out, const void *in, size_t n_in, Args&&... args) {
	Codec codec(std::forward<Args>(args)...);
	auto ibuf_ptr = static_cast<const uint8_t *>(in), ibuf_eptr = ibuf_ptr + n_in;
	bool finished;
	do {
		auto size = out.size();
		out.resize(size + OBufSize);
		auto obuf_ptr = reinterpret_cast<uint8_t *>(&out.front() + size);
		finished = codec.process(obuf_ptr, OBufSize, ibuf_ptr, ibuf_eptr - ibuf_ptr);
		out.resize(obuf_ptr - reinterpret_cast<uint8_t *>(&out.front()));
	} while (!finished);
	do {
		auto size = out.size();
		out.resize(size + OBufSize);
		auto obuf_ptr = reinterpret_cast<uint8_t *>(&out.front() + size);
		finished = codec.finish(obuf_ptr, OBufSize);
		out.resize(obuf_ptr - reinterpret_cast<uint8_t *>(&out.front()));
	} while (!finished);
	return out;
}

template <typename Codec, typename... Args>
static inline std::string & transcode(std::string &out, std::string_view in, Args&&... args) {
	return ::transcode<Codec, Args...>(out, in.data(), in.size(), std::forward<Args>(args)...);
}

template <typename Codec, size_t OBufSize = 512>
static inline std::string transcode(const void *in, size_t n_in) {
	std::string ret;
	::transcode<Codec, OBufSize>(ret, in, n_in);
	return ret;
}

template <typename Codec, size_t OBufSize = 512>
static inline std::string transcode(std::string_view in) {
	std::string ret;
	::transcode<Codec, OBufSize>(ret, in.data(), in.size());
	return ret;
}
