#include "codec.h"

#include <cassert>
#include <cstring>


template <typename Codec, size_t BufferSize>
ssize_t CodecSource<Codec, BufferSize>::read(void *buf, size_t n) {
	std::byte *obuf_ptr = static_cast<std::byte *>(buf), *obuf_eptr = obuf_ptr + n;
	ssize_t s = source.read(ibuf_eptr, std::end(ibuf) - ibuf_eptr);
	if (s > 0) {
		ibuf_eptr += s;
	}
	if (codec.process(obuf_ptr, n, const_cast<const std::byte *&>(ibuf_ptr), ibuf_eptr - ibuf_ptr)) {
		assert(ibuf_ptr == ibuf_eptr);
		ibuf_eptr = ibuf_ptr = ibuf;
		if (s < 0) {
			bool finished = codec.finish(obuf_eptr, obuf_eptr - obuf_ptr);
			s = obuf_ptr - static_cast<std::byte *>(buf);
			return s == 0 ? finished ? -1 : 0 : s;
		}
	}
	return obuf_ptr - static_cast<std::byte *>(buf);
}


template <typename Codec, size_t BufferSize>
size_t CodecSink<Codec, BufferSize>::write(const void *buf, size_t n) {
	const std::byte *ibuf_ptr = static_cast<const std::byte *>(buf);
	codec.process(obuf_eptr, std::end(obuf) - obuf_eptr, ibuf_ptr, n);
	if ((obuf_ptr += sink.write(obuf_ptr, obuf_eptr - obuf_ptr)) == obuf_eptr) {
		obuf_eptr = obuf_ptr = obuf;
	}
	return ibuf_ptr - static_cast<const std::byte *>(buf);
}

template <typename Codec, size_t BufferSize>
bool CodecSink<Codec, BufferSize>::flush() {
	for (;;) {
		bool finished = codec.finish(obuf_eptr, std::end(obuf) - obuf_eptr);
		if ((obuf_ptr += sink.write(obuf_ptr, obuf_eptr - obuf_ptr)) < obuf_eptr) {
			return false;
		}
		obuf_eptr = obuf_ptr = obuf;
		if (finished) {
			return sink.flush();
		}
	}
}
