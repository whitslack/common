#include <iostream>
#include <sstream>


class LogBuf : public std::stringbuf {
	friend class LogStream;

private:
	std::ostream *stream_ptr;

public:
	LogBuf(const LogBuf &copy) : std::stringbuf(copy.str()), stream_ptr(copy.stream_ptr) { }

private:
	LogBuf(std::ostream *stream_ptr) : stream_ptr(stream_ptr) { }

protected:
	int sync() override;

};


class LogStream : public std::ostream {
	friend class Log;

private:
	LogBuf buf;

public:
	LogStream(const LogStream &copy) : std::ios(), std::ostream(&buf), buf(copy.buf) { }

private:
	LogStream(std::ostream *stream_ptr, const char label[]);

};


class Log {

public:
	enum Level {
		SILENT, FATAL, ERROR, WARN, INFO, DEBUG, TRACE
	};

public:
	std::ostream * trace_stream_ptr;
	std::ostream * debug_stream_ptr;
	std::ostream * info_stream_ptr;
	std::ostream * warn_stream_ptr;
	std::ostream * error_stream_ptr;
	std::ostream * fatal_stream_ptr;

private:
	static const char trace_label[];
	static const char debug_label[];
	static const char info_label[];
	static const char warn_label[];
	static const char error_label[];
	static const char fatal_label[];

public:
	Log(Level level) : Log(level < TRACE ? nullptr : &std::clog, level < DEBUG ? nullptr : &std::clog, level < INFO ? nullptr : &std::clog, level < WARN ? nullptr : &std::clog, level < ERROR ? nullptr : &std::clog, level < FATAL ? nullptr : &std::clog) { }
	Log(std::ostream *trace_stream_ptr, std::ostream *debug_stream_ptr, std::ostream *info_stream_ptr, std::ostream *warn_stream_ptr, std::ostream *error_stream_ptr, std::ostream *fatal_stream_ptr) : trace_stream_ptr(trace_stream_ptr), debug_stream_ptr(debug_stream_ptr), info_stream_ptr(info_stream_ptr), warn_stream_ptr(warn_stream_ptr), error_stream_ptr(error_stream_ptr), fatal_stream_ptr(fatal_stream_ptr) { }
	bool trace_enabled() const { return trace_stream_ptr; }
	bool debug_enabled() const { return debug_stream_ptr; }
	bool info_enabled() const { return info_stream_ptr; }
	bool warn_enabled() const { return warn_stream_ptr; }
	bool error_enabled() const { return error_stream_ptr; }
	bool fatal_enabled() const { return fatal_stream_ptr; }
	LogStream trace() { return LogStream(trace_stream_ptr, trace_label); }
	LogStream debug() { return LogStream(debug_stream_ptr, debug_label); }
	LogStream info() { return LogStream(info_stream_ptr, info_label); }
	LogStream warn() { return LogStream(warn_stream_ptr, warn_label); }
	LogStream error() { return LogStream(error_stream_ptr, error_label); }
	LogStream fatal() { return LogStream(fatal_stream_ptr, fatal_label); }

};
