#pragma once

#include <array>
#include <streambuf>
#include <string_view>

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
	uintmax_t remaining;

private:
	Source &source;

public:
	constexpr LimitedSource(Source &source, uintmax_t remaining) noexcept : remaining(remaining), source(source) { }

public:
	_nodiscard ssize_t read(void *buf, size_t n) override;
	size_t avail() override { return remaining; }

};


class LimitedSink : public Sink {

public:
	uintmax_t remaining;

private:
	Sink &sink;

public:
	constexpr LimitedSink(Sink &sink, uintmax_t remaining) noexcept : remaining(remaining), sink(sink) { }

public:
	_nodiscard size_t write(const void *buf, size_t n) override;
	size_t avail() { return remaining; }

};


_nodiscard ssize_t read(void *dst, ConstBufferView &src, size_t n) noexcept;
_nodiscard size_t write(BufferView &dst, const void *src, size_t n) noexcept;


class MemorySource : public ConstBufferView, public Source {

public:
	constexpr MemorySource(const void *buf, size_t n) noexcept : ConstBufferView(static_cast<const std::byte *>(buf), static_cast<const std::byte *>(buf), static_cast<const std::byte *>(buf) + n, static_cast<const std::byte *>(buf) + n) { }

	template <typename R>
	constexpr explicit MemorySource(R &&range) noexcept : MemorySource(std::data(std::forward<R>(range)), std::size(std::forward<R>(range)) * sizeof(typename std::remove_reference_t<R>::value_type)) { }

public:
	_nodiscard ssize_t read(void *buf, size_t n) override { return ::read(buf, *this, n); }
	size_t _pure avail() override { return this->grem(); }

};


class MemorySink : public BufferView, public Sink {

public:
	constexpr MemorySink(void *buf, size_t n) noexcept : BufferView(static_cast<std::byte *>(buf), n) { }

public:
	_nodiscard size_t write(const void *buf, size_t n) override { return ::write(*this, buf, n); }

};


class StringSink : public Sink {

private:
	std::string &string;

public:
	constexpr explicit StringSink(std::string &string) noexcept : string(string) { }

public:
	_nodiscard size_t write(const void *buf, size_t n) override { return string.append(static_cast<const char *>(buf), n), n; }

};


class BufferViewSource : public Source {

private:
	ConstBufferView &buffer;

public:
	constexpr explicit BufferViewSource(ConstBufferView &buffer) noexcept : buffer(buffer) { }

public:
	_nodiscard ssize_t read(void *buf, size_t n) override { return ::read(buf, buffer, n); };

};


class BufferViewSink : public Sink {

private:
	BufferView &buffer;

public:
	constexpr explicit BufferViewSink(BufferView &buffer) noexcept : buffer(buffer) { }

public:
	_nodiscard size_t write(const void *buf, size_t n) override { return ::write(buffer, buf, n); }

};


class DynamicBufferSink : public DynamicBuffer, public Sink {

private:
	DynamicBuffer &buffer;

public:
	constexpr explicit DynamicBufferSink(DynamicBuffer &buffer) noexcept : buffer(buffer) { }

public:
	_nodiscard size_t write(const void *buf, size_t n) override { return buffer.append(buf, n), n; }

};


class BufferedSource : public Source, protected DynamicBuffer {

protected:
	Source &source;

public:
	explicit BufferedSource(Source &source) : BufferedSource(source, 8192) { }

	template <typename... Args>
	explicit BufferedSource(Source &source, Args &&...args) : DynamicBuffer(std::forward<Args>(args)...), source(source) { }

public:
	_nodiscard ssize_t read(void *buf, size_t n) override;
	size_t avail() override { return this->grem(); }

};


class BufferedSink : public Sink, protected DynamicBuffer {

protected:
	Sink &sink;

public:
	explicit BufferedSink(Sink &sink) : BufferedSink(sink, 8192) { }

	template <typename... Args>
	explicit BufferedSink(Sink &sink, Args &&...args) : DynamicBuffer(std::forward<Args>(args)...), sink(sink) { }

public:
	_nodiscard size_t write(const void *buf, size_t n) override;
	bool flush() override;

};


class DelimitedSource : public Source {

private:
	Source &source;
	const std::string_view delimiter;
	std::string_view::const_iterator delim_itr;

public:
	DelimitedSource(Source &source, std::string_view delimiter) noexcept : source(source), delimiter(delimiter), delim_itr(delimiter.begin()) { }

public:
	void reset() noexcept { delim_itr = delimiter.begin(); }
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
