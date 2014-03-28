#pragma once

#include <array>
#include <cstring>
#include <streambuf>

#include "compiler.h"


class Source {

public:
	virtual ~Source() { }

public:
	virtual ssize_t read(void *buf, size_t n) = 0;
	virtual size_t avail() { return 0; }

	void read_fully(void *buf, size_t n);

};


class Sink {

public:
	virtual ~Sink() { }

public:
	virtual size_t write(const void *buf, size_t n) = 0;
	virtual bool flush() { return true; }

	void write_fully(const void *buf, size_t n);
	void flush_fully();

};


template <typename Itr, typename E = typename std::iterator_traits<Itr>::value_type>
class IteratorSource : public Source {

public:
	Itr itr;

public:
	explicit IteratorSource(Itr itr) : itr(itr) { }

public:
	ssize_t read(void *buf, size_t n) override {
		auto out_ptr = static_cast<E *>(buf), out_eptr = out_ptr + n / sizeof(E);
		while (out_ptr < out_eptr) {
			*out_ptr++ = *itr++;
		}
		return reinterpret_cast<uint8_t *>(out_ptr) - static_cast<uint8_t *>(buf);
	}

};


template <typename Itr, typename E = typename std::iterator_traits<Itr>::value_type>
class IteratorSink : public Sink {

public:
	Itr itr;

public:
	explicit IteratorSink(Itr itr) : itr(itr) { }

public:
	size_t write(const void *buf, size_t n) override {
		auto in_ptr = static_cast<const E *>(buf), in_eptr = in_ptr + n / sizeof(E);
		while (in_ptr < in_eptr) {
			*itr++ = *in_ptr++;
		}
		return reinterpret_cast<const uint8_t *>(in_ptr) - static_cast<const uint8_t *>(buf);
	}

};


class LimitedSource : public Source {

public:
	size_t remaining;

private:
	Source &source;

public:
	LimitedSource(Source &source, size_t remaining) : remaining(remaining), source(source) { }

public:
	ssize_t read(void *buf, size_t n) override;
	size_t avail() override { return remaining; }

};


class LimitedSink : public Sink {

public:
	size_t remaining;

private:
	Sink &sink;

public:
	LimitedSink(Sink &sink, size_t remaining) : remaining(remaining), sink(sink) { }

public:
	size_t write(const void *buf, size_t n) override;
	size_t avail() { return remaining; }

};


class MemorySource : public LimitedSource {

private:
	IteratorSource<const uint8_t *> source;

public:
	MemorySource(const void *buf, size_t n) : LimitedSource(source, n), source(static_cast<const uint8_t *>(buf)) { }

public:
	const void * data() const { return source.itr; }

};


class MemorySink : public LimitedSink {

private:
	IteratorSink<uint8_t *> sink;

public:
	MemorySink(void *buf, size_t n) : LimitedSink(sink, n), sink(static_cast<uint8_t *>(buf)) { }

public:
	void * data() const { return sink.itr; }

};


template <typename C, typename E = typename C::value_type>
class ContainerSource : public LimitedSource {

private:
	IteratorSource<typename C::const_iterator, E> source;

public:
	explicit ContainerSource(const C &c) : LimitedSource(source, c.size()), source(c.begin()) { }

};


template <typename C, typename E = typename C::value_type>
class ContainerSink : public IteratorSink<std::back_insert_iterator<C>, E> {

public:
	explicit ContainerSink(C &c) : IteratorSink<std::back_insert_iterator<C>, E>(std::back_inserter(c)) { }

};


typedef ContainerSource<std::string> StringSource;
typedef ContainerSink<std::string> StringSink;


class BufferedSourceBase : public Source {

private:
	Source &source;
	uint8_t * const buf_bptr, *buf_gptr, *buf_pptr, * const buf_eptr;

protected:
	BufferedSourceBase(Source &source, uint8_t *buf_bptr, uint8_t *buf_eptr) : source(source), buf_bptr(buf_bptr), buf_gptr(buf_bptr), buf_pptr(buf_bptr), buf_eptr(buf_eptr) { }

public:
	ssize_t read(void *buf, size_t n) override;
	size_t avail() override { return buf_pptr - buf_gptr; }

};


class BufferedSinkBase : public Sink {

private:
	Sink &sink;
	uint8_t * const buf_bptr, *buf_gptr, *buf_pptr, * const buf_eptr;

protected:
	BufferedSinkBase(Sink &sink, uint8_t *buf_bptr, uint8_t *buf_eptr) : sink(sink), buf_bptr(buf_bptr), buf_gptr(buf_bptr), buf_pptr(buf_bptr), buf_eptr(buf_eptr) { }

public:
	size_t write(const void *buf, size_t n) override;
	bool flush() override;

};


template <size_t Buffer_Size>
class BufferedSource : public BufferedSourceBase {

private:
	std::array<uint8_t, Buffer_Size> buffer;

public:
	explicit BufferedSource(Source &source) : BufferedSourceBase(source, &*buffer.begin(), &*buffer.end()) { }

};


template <size_t Buffer_Size>
class BufferedSink : public BufferedSinkBase {

private:
	std::array<uint8_t, Buffer_Size> buffer;

public:
	explicit BufferedSink(Sink &sink) : BufferedSinkBase(sink, &*buffer.begin(), &*buffer.end()) { }

};


class DelimitedSource : public Source {

private:
	Source &source;
	const char * const delim_begin, * const delim_end;
	const char *delim_ptr;

public:
	DelimitedSource(Source &source, const char delimiter[], const char *delim_end) : source(source), delim_begin(delimiter), delim_end(delim_end), delim_ptr(delimiter) { }
	DelimitedSource(Source &source, const char delimiter[]) : DelimitedSource(source, delimiter, delimiter + std::strlen(delimiter)) { }

public:
	void reset() { delim_ptr = delim_begin; }
	ssize_t read(void *buf, size_t n) override;

};


class Tap : public Source {

private:
	Source &source;
	Sink &sink;

public:
	Tap(Source &source, Sink &sink) : source(source), sink(sink) { }

public:
	ssize_t read(void *buf, size_t n) override;
	size_t avail() { return source.avail(); }

};


template <size_t N>
class Tee : public Sink {

private:
	const std::array<Sink *, N> sinks;

public:
	explicit Tee(const std::array<Sink *, N> &sinks) : sinks(sinks) { }

	template <typename... Args>
	explicit Tee(Args&&... args) : sinks({ args... }) { }

public:
	size_t write(const void *buf, size_t n) override {
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


class SourceBuf : public virtual std::streambuf {

protected:
	Source &source;

private:
	char_type gbuf;

public:
	explicit SourceBuf(Source &source);
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
	explicit SinkBuf(Sink &sink) : sink(sink) { }
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
	explicit SourceSinkBuf(T &source_sink) : SourceBuf(source_sink), SinkBuf(source_sink) { }

	SourceSinkBuf(Source &source, Sink &sink) : SourceBuf(source), SinkBuf(sink) { }

protected:
	std::streambuf * setbuf(char_type s[], std::streamsize n) override;

};


class MemoryBuf : public std::streambuf {

public:
	MemoryBuf(const void *buf, size_t n);

protected:
	MemoryBuf * setbuf(char s[], std::streamsize n) override;

};
