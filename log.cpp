#include "log.h"

#include <ctime>
#include <iomanip>


const char Log::trace_label[] = "TRACE";
const char Log::debug_label[] = "DEBUG";
const char Log::info_label[] = "INFO";
const char Log::warn_label[] = "WARN";
const char Log::error_label[] = "ERROR";
const char Log::fatal_label[] = "FATAL";

int LogBuf::sync() {
	auto ret = this->std::stringbuf::sync();
	if (stream_ptr) {
		auto str = this->str();
		stream_ptr->write(str.data(), str.size()).flush();
	}
	return ret;
}

LogStream::LogStream(std::ostream *stream_ptr, const char label[]) : std::ostream(stream_ptr ? &buf : nullptr), buf(stream_ptr) {
	if (stream_ptr) {
		std::time_t time = std::time(nullptr);
		std::tm tm;
		::localtime_r(&time, &tm);
		*this << std::put_time(&tm, "%c") << "  " << std::setw(5) << label << "  ";
	}
}
