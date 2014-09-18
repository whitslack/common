#include "log.h"

#include <ctime>


const char Log::trace_label[] = "TRACE";
const char Log::debug_label[] = "DEBUG";
const char Log::info_label[] = "INFO";
const char Log::warn_label[] = "WARN";
const char Log::error_label[] = "ERROR";
const char Log::fatal_label[] = "FATAL";

LogStream::LogStream(std::ostream *stream_ptr, const char label[]) : std::ostream(&sb), ss(str), sb(ss), stream_ptr(stream_ptr) {
	if (stream_ptr) {
		time_t time = ::time(nullptr);
		std::tm tm;
		::localtime_r(&time, &tm);
		// [C++11] *this << std::put_time(&tm, "%c") << "  " << std::setw(5) << label << "  ";
		char buf[34];
		size_t n = std::strftime(buf, sizeof buf, "%c", &tm);
		n += std::snprintf(buf + n, sizeof buf - n, "  %5s  ", label);
		str.append(buf, n);
	}
}

LogStream::~LogStream() {
	if (stream_ptr) {
		stream_ptr->write(str.data(), str.size()).flush();
	}
}
