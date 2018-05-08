#pragma once

#include <array>
#include <cstring>
#include <streambuf>
#include <vector>

#include "buffer.h"
#include "compiler.h"


class Source {

public:
	struct BufferPointer {
		void *ptr;
		size_t size;
	};

public:
	virtual ~Source() = default;

public:
	_nodiscard virtual ssize_t read(void *buf, size_t n) = 0;
	_nodiscard virtual ssize_t read(const BufferPointer bufs[], size_t count);
	virtual size_t avail() { return 0; }

	void read_fully(void *buf, size_t n);
	void read_fully(const BufferPointer bufs[], size_t count);

	_nodiscard ssize_t read(std::initializer_list<BufferPointer> bufs) { return this->read(bufs.begin(), bufs.size()); }
	void read_fully(std::initializer_list<BufferPointer> bufs) { return this->read_fully(bufs.begin(), bufs.size()); }

};


class Sink {

public:
	struct BufferPointer {
		const void *ptr;
		size_t size;
	};

public:
	virtual ~Sink() = default;

public:
	_nodiscard virtual size_t write(const void *buf, size_t n) = 0;
	_nodiscard virtual size_t write(const BufferPointer bufs[], size_t count);
	virtual bool flush() { return true; }

	void write_fully(const void *buf, size_t n);
	void write_fully(const BufferPointer bufs[], size_t count);
	void flush_fully();

	_nodiscard size_t write(std::initializer_list<BufferPointer> bufs) { return this->write(bufs.begin(), bufs.size()); }
	void write_fully(std::initializer_list<BufferPointer> bufs) { return this->write_fully(bufs.begin(), bufs.size()); }

};


class LimitedSource : public Source {

public:
	size_t remaining;

private:
	Source &source;

public:
	LimitedSource(Source &source, size_t remaining) noexcept : remaining(remaining), source(source) { }

public:
	_nodiscard ssize_t read(void *buf, size_t n) override;
	size_t avail() override { return remaining; }

};


class LimitedSink : public Sink {

public:
	size_t remaining;

private:
	Sink &sink;

public:
	LimitedSink(Sink &sink, size_t remaining) noexcept : remaining(remaining), sink(sink) { }

public:
	_nodiscard size_t write(const void *buf, size_t n) override;
	size_t avail() { return remaining; }

};


class MemorySource : public BasicStaticBuffer<const uint8_t>, public Source {

public:
	MemorySource(const void *buf, size_t n) noexcept : BasicStaticBuffer<const uint8_t>(static_cast<const uint8_t *>(buf), static_cast<const uint8_t *>(buf), static_cast<const uint8_t *>(buf) + n, static_cast<const uint8_t *>(buf) + n) { }

public:
	_nodiscard ssize_t read(void *buf, size_t n) override;
	size_t _pure avail() override { return this->grem(); }

};


class MemorySink : public StaticBuffer, public Sink {

public:
	MemorySink(void *buf, size_t n) noexcept : StaticBuffer(static_cast<uint8_t *>(buf), n) { }

public:
	_nodiscard size_t write(const void *buf, size_t n) override;

};


class BufferSource : public Source {

private:
	StaticBuffer &buffer;

public:
	explicit BufferSource(StaticBuffer &buffer) noexcept : buffer(buffer) { }

public:
	_nodiscard ssize_t read(void *buf, size_t n) override;
	size_t _pure avail() override { return buffer.grem(); }

};


class StaticBufferSink : public Sink {

private:
	StaticBuffer &buffer;

public:
	explicit StaticBufferSink(StaticBuffer &buffer) noexcept : buffer(buffer) { }

public:
	_nodiscard size_t write(const void *buf, size_t n) override;

};


class BufferSink : public Buffer, public Sink {

public:
	BufferSink() noexcept = default;
	explicit BufferSink(size_t initial_size) : Buffer(initial_size) { }
	explicit BufferSink(Buffer &&move) : Buffer(std::move(move)) { }

public:
	_nodiscard size_t write(const void *buf, size_t n) override;

};


class StringSource : public MemorySource {

public:
	template <typename CharT, typename Traits>
	explicit StringSource(std::basic_string_view<CharT, Traits> sv) noexcept : MemorySource(sv.data(), sv.size() * sizeof(CharT)) { }

};


class StringSink : public Sink {

private:
	std::string &string;

public:
	explicit StringSink(std::string &string) noexcept : string(string) { }

public:
	_nodiscard size_t write(const void *buf, size_t n) override;

};


class BufferedSourceBase : public Source {

private:
	Source &source;
	uint8_t * const buf_bptr, *buf_gptr, *buf_pptr, * const buf_eptr;

protected:
	explicit BufferedSourceBase(Source &source, uint8_t *buf_bptr, uint8_t *buf_eptr) noexcept : source(source), buf_bptr(buf_bptr), buf_gptr(buf_bptr), buf_pptr(buf_bptr), buf_eptr(buf_eptr) { }

public:
	_nodiscard ssize_t read(void *buf, size_t n) override;
	size_t avail() override { return buf_pptr - buf_gptr; }

};


class BufferedSinkBase : public Sink {

private:
	Sink &sink;
	uint8_t * const buf_bptr, *buf_gptr, *buf_pptr, * const buf_eptr;

protected:
	explicit BufferedSinkBase(Sink &sink, uint8_t *buf_bptr, uint8_t *buf_eptr) noexcept : sink(sink), buf_bptr(buf_bptr), buf_gptr(buf_eptr), buf_pptr(buf_eptr), buf_eptr(buf_eptr) { }

public:
	_nodiscard size_t write(const void *buf, size_t n) override;
	bool flush() override;

};


template <size_t Buffer_Size>
class BufferedSource : public BufferedSourceBase {

private:
	std::array<uint8_t, Buffer_Size> buffer;

public:
	explicit BufferedSource(Source &source) noexcept : BufferedSourceBase(source, &*buffer.begin(), &*buffer.end()) { }

};


template <size_t Buffer_Size>
class BufferedSink : public BufferedSinkBase {

private:
	std::array<uint8_t, Buffer_Size> buffer;

public:
	explicit BufferedSink(Sink &sink) noexcept : BufferedSinkBase(sink, &*buffer.begin(), &*buffer.end()) { }

};


class DelimitedSource : public Source {

private:
	Source &source;
	const char * const delim_begin, * const delim_end;
	const char *delim_ptr;

public:
	DelimitedSource(Source &source, const char delimiter[], const char *delim_end) noexcept : source(source), delim_begin(delimiter), delim_end(delim_end), delim_ptr(delimiter) { }
	DelimitedSource(Source &source, const char delimiter[]) noexcept : DelimitedSource(source, delimiter, delimiter + std::strlen(delimiter)) { }

public:
	void reset() noexcept { delim_ptr = delim_begin; }
	_nodiscard ssize_t read(void *buf, size_t n) override;

};


class Tap : public Source {

private:
	Source &source;
	Sink &sink;

public:
	Tap(Source &source, Sink &sink) noexcept : source(source), sink(sink) { }

public:
	_nodiscard ssize_t read(void *buf, size_t n) override;
	size_t avail() override { return source.avail(); }

};


template <size_t N>
class Tee : public Sink {

private:
	const std::array<Sink *, N> sinks;

public:
	explicit Tee(const std::array<Sink *, N> &sinks) noexcept : sinks(sinks) { }

	template <typename... Args>
	explicit Tee(Args&&... args) noexcept : sinks({ std::forward<Args>(args)... }) { }

public:
	_nodiscard size_t write(const void *buf, size_t n) override {
		for (auto sink : sinks) {
			sink->write_fully(buf, n);
		}
		return n;
	}

	bool flush() override {
		bool ret = true;
		for (auto sink : sinks) {
			ret &= sink->flush();
		}
		return ret;
	}

};


class StreamBufSourceSink : public Source, public Sink {

private:
	std::streambuf &sb;

public:
	explicit StreamBufSourceSink(std::streambuf &sb) noexcept : sb(sb) { }

public:
	_nodiscard ssize_t read(void *buf, size_t n) override;
	size_t avail() override;

	_nodiscard size_t write(const void *buf, size_t n) override;
	bool flush() override { return sb.pubsync() == 0; }

};


class SourceBuf : public virtual std::streambuf {

protected:
	Source &source;

private:
	char_type gbuf;

public:
	explicit SourceBuf(Source &source) noexcept;
	SourceBuf(const SourceBuf &) = delete;
	SourceBuf & operator = (const SourceBuf &) = delete;
	SourceBuf(SourceBuf &&) = default;
	SourceBuf & operator = (SourceBuf &&) = default;

protected:
	std::streambuf * setbuf(char_type s[], std::streamsize n) override;
	std::streamsize showmanyc() override;
	int_type underflow() override;
	std::streamsize xsgetn(char_type s[], std::streamsize n) override;

};


class SinkBuf : public virtual std::streambuf {

protected:
	Sink &sink;

public:
	explicit SinkBuf(Sink &sink) noexcept : sink(sink) { }
	SinkBuf(const SinkBuf &) = delete;
	SinkBuf & operator = (const SinkBuf &) = delete;
	SinkBuf(SinkBuf &&) = default;
	SinkBuf & operator = (SinkBuf &&) = default;

protected:
	std::streambuf * setbuf(char_type s[], std::streamsize n) override;
	int sync() override;
	int_type overflow(int_type ch = traits_type::eof()) override;
	std::streamsize xsputn(const char_type s[], std::streamsize n) override;

private:
	int sync(bool more);

};


class SourceSinkBuf : public SourceBuf, public SinkBuf {

public:
	template <typename T>
	explicit SourceSinkBuf(T &source_sink) noexcept : SourceBuf(source_sink), SinkBuf(source_sink) { }

	SourceSinkBuf(Source &source, Sink &sink) noexcept : SourceBuf(source), SinkBuf(sink) { }

protected:
	std::streambuf * setbuf(char_type s[], std::streamsize n) override;

};


class MemoryBuf : public std::streambuf {

public:
	MemoryBuf(const void *buf, size_t n);

protected:
	MemoryBuf * setbuf(char s[], std::streamsize n) override;

};
