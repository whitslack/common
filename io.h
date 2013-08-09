#pragma once

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
	virtual size_t write(const void *buf, size_t n, bool more = false) = 0;

	void write_fully(const void *buf, size_t n, bool more = false);

};


class StringSource : public Source {

private:
	const std::string *string;
	std::string::const_iterator string_itr;

public:
	StringSource(const std::string *string) : string(string), string_itr(string->begin()) { }

public:
	ssize_t read(void *buf, size_t n) override;
	size_t avail() override _pure;

};


class StringSink : public Sink {

private:
	std::string *string;

public:
	StringSink(std::string *string) : string(string) { }

public:
	size_t write(const void *buf, size_t n, bool more = false) override;

};


class SourceBuf : public virtual std::streambuf {

protected:
	Source *source;

private:
	char_type gbuf;

public:
	explicit SourceBuf(Source *source);
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
	Sink *sink;

public:
	explicit SinkBuf(Sink *sink) : sink(sink) { }
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
	explicit SourceSinkBuf(T *source_sink) : SourceBuf(source_sink), SinkBuf(source_sink) { }
	SourceSinkBuf(Source *source, Sink *sink) : SourceBuf(source), SinkBuf(sink) { }

protected:
	std::streambuf * setbuf(char_type s[], std::streamsize n) override;

};
