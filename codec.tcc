#include "codec.h"

#include <cstring>


template <typename Codec>
ssize_t CodecSource<Codec>::read(void *buf, size_t n) {
	ssize_t ret = 0;
	bool more = true;
	for (;;) {
		size_t r;
		if ((r = osize - opos) > 0) {
			if (n < r) {
				r = n;
			}
			std::memcpy(buf, obuf + opos, r), opos += r;
			buf = static_cast<uint8_t *>(buf) + r, n -= r, ret += r;
			if (n == 0) {
				return ret;
			}
		}
		while ((r = sizeof ibuf - ipos) > 0) {
			ssize_t s = source->read(ibuf + ipos, r);
			if (s == 0) {
				return ret;
			}
			if (s < 0) {
				if (!more) {
					return ret == 0 ? -1 : ret;
				}
				more = false;
				break;
			}
			ipos += s;
		}
		if (n < sizeof obuf) {
			osize = codec.process(obuf, ibuf, ipos), opos = ipos = 0;
		}
		else {
			r = codec.process(*static_cast<uint8_t (*)[sizeof obuf]>(buf), ibuf, ipos), ipos = 0;
			buf = static_cast<uint8_t *>(buf) + r, n -= r, ret += r;
		}
	}
}

template <typename Codec>
size_t CodecSource<Codec>::avail() {
	return osize - opos;
}


template <typename Codec>
size_t CodecSink<Codec>::write(const void *buf, size_t n, bool more) {
	size_t ret = 0;
	for (;;) {
		for (size_t r; (r = osize - opos) > 0; opos += r) {
			if ((r = sink->write(obuf + opos, r)) == 0) {
				return ret;
			}
		}
		if (ipos > 0 || n < sizeof ibuf) {
			size_t r = sizeof ibuf - ipos;
			if (n < r) {
				r = n;
			}
			std::memcpy(ibuf + ipos, buf, r), ipos += r;
			buf = static_cast<const uint8_t *>(buf) + r, n -= r, ret += r;
			if (n == 0) {
				if (more) {
					return ret;
				}
				more = true;
			}
			osize = codec.process(obuf, ibuf, ipos), opos = ipos = 0;
		}
		else {
			osize = codec.process(obuf, static_cast<const uint8_t *>(buf), sizeof ibuf), opos = 0;
			buf = static_cast<const uint8_t *>(buf) + sizeof ibuf, n -= sizeof ibuf, ret += sizeof ibuf;
		}
	}
}
