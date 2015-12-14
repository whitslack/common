#include <tuple>

#include "io.h"


template <size_t S> struct _varint_ops { };
template <> struct _varint_ops<4> {
	typedef int32_t signed_type;
	typedef uint32_t unsigned_type;
	static Source & read_signed(Source &source, signed_type &value);
	static Sink & write_signed(Sink &sink, signed_type value);
	static Source & read_unsigned(Source &source, unsigned_type &value);
	static Sink & write_unsigned(Sink &sink, unsigned_type value);
};
template <> struct _varint_ops<8> {
	typedef int64_t signed_type;
	typedef uint64_t unsigned_type;
	static Source & read_signed(Source &source, signed_type &value);
	static Sink & write_signed(Sink &sink, signed_type value);
	static Source & read_unsigned(Source &source, unsigned_type &value);
	static Sink & write_unsigned(Sink &sink, unsigned_type value);
};

template <typename T>
static inline typename std::enable_if<std::is_integral<T>::value && std::is_signed<T>::value, Source &>::type read_varint(Source &source, T &value) {
	return _varint_ops<sizeof(T)>::read_signed(source, reinterpret_cast<typename _varint_ops<sizeof(T)>::signed_type &>(value));
}

template <typename T>
static inline typename std::enable_if<std::is_integral<T>::value && std::is_signed<T>::value, Sink &>::type write_varint(Sink &sink, T value) {
	return _varint_ops<sizeof(T)>::write_signed(sink, value);
}

template <typename T>
static inline typename std::enable_if<std::is_integral<T>::value && std::is_unsigned<T>::value, Source &>::type read_varint(Source &source, T &value) {
	return _varint_ops<sizeof(T)>::read_unsigned(source, reinterpret_cast<typename _varint_ops<sizeof(T)>::unsigned_type &>(value));
}

template <typename T>
static inline typename std::enable_if<std::is_integral<T>::value && std::is_unsigned<T>::value, Sink &>::type write_varint(Sink &sink, T value) {
	return _varint_ops<sizeof(T)>::write_unsigned(sink, value);
}

template <typename T>
struct _varint {
	T x;
};

template <typename T>
static inline typename std::enable_if<std::is_integral<T>::value, _varint<T &>>::type varint(T &x) {
	return { x };
}

template <typename T>
static inline typename std::enable_if<std::is_integral<T>::value, _varint<const T>>::type varint(const T &x) {
	return { x };
}

template <typename T>
static inline _varint<typename std::underlying_type<T>::type &> varenum(T &ref) {
	return varint(reinterpret_cast<typename std::underlying_type<T>::type &>(ref));
}

template <typename T>
static inline _varint<const typename std::underlying_type<T>::type> varenum(const T &ref) {
	return varint(reinterpret_cast<const typename std::underlying_type<T>::type &>(ref));
}

template <typename T>
static inline Source & operator >> (Source &source, _varint<T> varint) {
	return read_varint(source, varint.x);
}

template <typename T>
static inline Sink & operator << (Sink &sink, _varint<T> varint) {
	return write_varint(sink, varint.x);
}

template <typename T>
static inline T read_varint(Source &source) {
	T value;
	read_varint(source, value);
	return value;
}

template <typename T>
static inline typename std::enable_if<std::is_trivially_copyable<T>::value, Source>::type & operator >> (Source &source, T &value) {
	source.read_fully(&value, sizeof value);
	return source;
}

template <typename T>
static inline typename std::enable_if<std::is_trivially_copyable<T>::value, Sink>::type & operator << (Sink &sink, const T &value) {
	sink.write_fully(&value, sizeof value);
	return sink;
}

template <typename T>
static inline typename std::enable_if<std::is_trivially_copyable<T>::value, Source>::type & operator >> (Source &source, std::basic_string<T> &string) {
	string.resize(read_varint<size_t>(source));
	source.read_fully(&string.front(), string.size() * sizeof(T));
	return source;
}

template <typename T>
static inline typename std::enable_if<std::is_trivially_copyable<T>::value, Sink>::type & operator << (Sink &sink, const std::basic_string<T> &string) {
	write_varint(sink, string.size());
	sink.write_fully(string.data(), string.size() * sizeof(T));
	return sink;
}

template <typename T>
static inline typename std::enable_if<std::is_trivially_copyable<T>::value, Source>::type & operator >> (Source &source, std::vector<T> &vector) {
	vector.resize(read_varint<size_t>(source));
	source.read_fully(vector.data(), vector.size() * sizeof(T));
	return source;
}

template <typename T>
static inline typename std::enable_if<std::is_trivially_copyable<T>::value, Sink>::type & operator << (Sink &sink, const std::vector<T> &vector) {
	write_varint(sink, vector.size());
	sink.write_fully(vector.data(), vector.size() * sizeof(T));
	return sink;
}

template <typename T, size_t N>
static inline typename std::enable_if<!std::is_trivially_copyable<T>::value, Source>::type & operator >> (Source &source, std::array<T, N> &array) {
	for (auto &element : array) {
		source >> element;
	}
	return source;
}

template <typename T, size_t N>
static inline typename std::enable_if<!std::is_trivially_copyable<T>::value, Sink>::type & operator << (Sink &sink, const std::array<T, N> &array) {
	for (auto &element : array) {
		sink << element;
	}
	return sink;
}

template <typename T>
static inline typename std::enable_if<!std::is_trivially_copyable<T>::value, Source>::type & operator >> (Source &source, std::vector<T> &vector) {
	vector.resize(read_varint<size_t>(source));
	for (auto &element : vector) {
		source >> element;
	}
	return source;
}

template <typename T>
static inline typename std::enable_if<!std::is_trivially_copyable<T>::value, Sink>::type & operator << (Sink &sink, const std::vector<T> &vector) {
	write_varint(sink, vector.size());
	for (auto &element : vector) {
		sink << element;
	}
	return sink;
}

template <typename T1, typename T2>
static inline Source & operator >> (Source &source, std::pair<T1, T2> &pair) {
	return source >> pair.first >> pair.second;
}

template <typename T1, typename T2>
static inline Sink & operator << (Sink &sink, const std::pair<T1, T2> &pair) {
	return sink << pair.first << pair.second;
}

template <size_t I, typename... Types>
static inline typename std::enable_if<(I < sizeof...(Types)), Source>::type & read_tuple(Source &source, std::tuple<Types...> &tuple) {
	return read_tuple<I + 1>(source >> std::get<I>(tuple), tuple);
}

template <size_t I, typename... Types>
static inline typename std::enable_if<I == sizeof...(Types), Source>::type & read_tuple(Source &source, std::tuple<Types...> &tuple) {
	return source;
}

template <typename... Types>
static inline Source & operator >> (Source &source, std::tuple<Types...> &tuple) {
	return read_tuple<0>(source, tuple);
}

template <size_t I, typename... Types>
static inline typename std::enable_if<(I < sizeof...(Types)), Sink>::type & write_tuple(Sink &sink, const std::tuple<Types...> &tuple) {
	return write_tuple<I + 1>(sink << std::get<I>(tuple), tuple);
}

template <size_t I, typename... Types>
static inline typename std::enable_if<I == sizeof...(Types), Sink>::type & write_tuple(Sink &sink, const std::tuple<Types...> &tuple) {
	return sink;
}

template <typename... Types>
static inline Sink & operator << (Sink &sink, const std::tuple<Types...> &tuple) {
	return write_tuple<0>(sink, tuple);
}

template <typename T>
static inline std::vector<uint8_t> serialize(const T &value) {
	std::vector<uint8_t> buffer;
	VectorSink vs(buffer);
	vs << value;
	return buffer;
}

template <typename T>
static inline T deserialize(Source &source) {
	T value;
	source >> value;
	return value;
}

template <typename T>
static inline T deserialize(const void *buf, size_t buf_size) {
	MemorySource ms(buf, buf_size);
	return deserialize<T>(ms);
}
