#include "log.h"

#include <chrono>
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
		auto sv = this->view();
		stream_ptr->write(sv.data(), sv.size()).flush();
	}
	return ret;
}

LogStream::LogStream(std::ostream *stream_ptr, const char label[]) : std::ostream(stream_ptr ? &buf : nullptr), buf(stream_ptr) {
	if (stream_ptr) {
		auto now = std::chrono::system_clock::now();
		std::time_t time = std::chrono::system_clock::to_time_t(now);
		std::tm tm;
		::localtime_r(&time, &tm);
		*this << std::put_time(&tm, "%FT%T") << ','
				<< std::setfill('0') << std::setw(6) << std::chrono::duration_cast<std::chrono::microseconds>(now - std::chrono::time_point_cast<std::chrono::seconds>(now)).count()
				<< std::setfill(' ') << "  " << std::setw(5) << label << "  ";
	}
}
