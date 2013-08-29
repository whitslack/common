#define ZLIB_CONST

#include <zlib.h>

#include "io.h"


class GZipSource : public Source {

private:
	Source *source;
	z_stream stream;
	uint8_t ibuf[1 << 12];

public:
	explicit GZipSource(Source *source);
	~GZipSource();

private:
	GZipSource(const GZipSource &) = delete;
	GZipSource & operator = (const GZipSource &) = delete;

public:
	ssize_t read(void *buf, size_t n) override;

};


class GZipSink : public Sink {

private:
	Sink *sink;
	z_stream stream;
	uint8_t obuf[1 << 12];

public:
	explicit GZipSink(Sink *sink, int level = Z_DEFAULT_COMPRESSION);
	~GZipSink();

private:
	GZipSink(const GZipSink &) = delete;
	GZipSink & operator = (const GZipSink &) = delete;

public:
	size_t write(const void *buf, size_t n, bool more = false) override;
	bool finish() override;

private:
	int write(const void *buf, size_t &n, int flush);

};
