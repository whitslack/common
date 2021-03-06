#include <iostream>
#include <sstream>

#include "compiler.h"


class LogBuf : public std::stringbuf {
	friend class LogStream;

private:
	std::ostream *stream_ptr;

private:
	explicit LogBuf(std::ostream *stream_ptr) noexcept : stream_ptr(stream_ptr) { }

protected:
	int sync() override;

};


class LogStream : public std::ostream {
	friend class Log;

private:
	LogBuf buf;

private:
	explicit LogStream(std::ostream *stream_ptr, const char label[]);

};


class Log {

public:
	enum Level {
		SILENT, FATAL, ERROR, WARN, INFO, DEBUG, TRACE
	};

public:
	std::ostream *trace_stream_ptr, *debug_stream_ptr, *info_stream_ptr, *warn_stream_ptr, *error_stream_ptr, *fatal_stream_ptr;

private:
	static const char trace_label[], debug_label[], info_label[], warn_label[], error_label[], fatal_label[];

public:
	Log(Level level) : Log(level < TRACE ? nullptr : &std::clog, level < DEBUG ? nullptr : &std::clog, level < INFO ? nullptr : &std::clog, level < WARN ? nullptr : &std::clog, level < ERROR ? nullptr : &std::clog, level < FATAL ? nullptr : &std::clog) { }
	Log(std::ostream *trace_stream_ptr, std::ostream *debug_stream_ptr, std::ostream *info_stream_ptr, std::ostream *warn_stream_ptr, std::ostream *error_stream_ptr, std::ostream *fatal_stream_ptr) : trace_stream_ptr(trace_stream_ptr), debug_stream_ptr(debug_stream_ptr), info_stream_ptr(info_stream_ptr), warn_stream_ptr(warn_stream_ptr), error_stream_ptr(error_stream_ptr), fatal_stream_ptr(fatal_stream_ptr) { }

public:
	bool _pure trace_enabled() const noexcept { return trace_stream_ptr; }
	bool _pure debug_enabled() const noexcept { return debug_stream_ptr; }
	bool _pure info_enabled() const noexcept { return info_stream_ptr; }
	bool _pure warn_enabled() const noexcept { return warn_stream_ptr; }
	bool _pure error_enabled() const noexcept { return error_stream_ptr; }
	bool _pure fatal_enabled() const noexcept { return fatal_stream_ptr; }

	LogStream trace() { return LogStream(trace_stream_ptr, trace_label); }
	LogStream debug() { return LogStream(debug_stream_ptr, debug_label); }
	LogStream info() { return LogStream(info_stream_ptr, info_label); }
	LogStream warn() { return LogStream(warn_stream_ptr, warn_label); }
	LogStream error() { return LogStream(error_stream_ptr, error_label); }
	LogStream fatal() { return LogStream(fatal_stream_ptr, fatal_label); }

};
