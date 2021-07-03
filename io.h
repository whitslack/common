#pragma once

#include <array>
#include <functional>
#include <optional>
#include <streambuf>
#include <string_view>

#include "buffer.h"
#include "compiler.h"
#include "narrow.h"
#include "span.h"


struct BufferPointer {
	void *ptr;
	size_t size;
};


struct ConstBufferPointer {
	const void *ptr;
	size_t size;
};


template <typename Derived, typename... Args>
class Readable {

public:
	_nodiscard ssize_t read(std::span<const BufferPointer> bufs, Args &&...args);

	void read_fully(void *buf, size_t n, Args &&...args);
	void read_fully(std::span<const BufferPointer> bufs, Args &&...args);

	_nodiscard ssize_t read(std::initializer_list<BufferPointer> bufs, Args &&...args) { return static_cast<Derived *>(this)->read({ bufs }, std::forward<Args>(args)...); }
	void read_fully(std::initializer_list<BufferPointer> bufs, Args &&...args) { return static_cast<Derived *>(this)->read_fully({ bufs }, std::forward<Args>(args)...); }

	_deprecated _nodiscard ssize_t read(const BufferPointer bufs[], size_t count, Args &&...args) { return static_cast<Derived *>(this)->read({ bufs, count }, std::forward<Args>(args)...); }
	_deprecated void read_fully(const BufferPointer bufs[], size_t count, Args &&...args) { return static_cast<Derived *>(this)->read_fully({ bufs, count }, std::forward<Args>(args)...); }

protected:
	template <typename NewDerived, typename... NewArgs>
	constexpr auto _const recast() noexcept { return reinterpret_cast<Readable<NewDerived, NewArgs...> &>(*this); }

};


template <typename Derived, typename... Args>
class Writable {

public:
	_nodiscard size_t write(std::span<const ConstBufferPointer> bufs, Args &&...args);

	void write_fully(const void *buf, size_t n, Args &&...args);
	void write_fully(std::span<const ConstBufferPointer> bufs, Args &&...args);

	_nodiscard size_t write(std::initializer_list<ConstBufferPointer> bufs, Args &&...args) { return static_cast<Derived *>(this)->write({ bufs }, std::forward<Args>(args)...); }
	void write_fully(std::initializer_list<ConstBufferPointer> bufs, Args &&...args) { return static_cast<Derived *>(this)->write_fully({ bufs }, std::forward<Args>(args)...); }

	_deprecated _nodiscard size_t write(const ConstBufferPointer bufs[], size_t count, Args &&...args) { return static_cast<Derived *>(this)->write({ bufs, count }, std::forward<Args>(args)...); }
	_deprecated void write_fully(const ConstBufferPointer bufs[], size_t count, Args &&...args) { return static_cast<Derived *>(this)->write_fully({ bufs, count }, std::forward<Args>(args)...); }

protected:
	template <typename NewDerived, typename... NewArgs>
	constexpr auto _const recast() noexcept { return reinterpret_cast<Writable<NewDerived, NewArgs...> &>(*this); }

};


template <typename Derived, typename... Args>
class Flushable {

public:
	template <typename T = Derived>
	static std::enable_if_t<std::is_convertible_v<decltype(std::declval<T>().flush(std::declval<Args>()...)), bool>, std::optional<bool>> flush_if_supported(Derived &derived, Args &&...args) {
		return derived.flush(std::forward<Args>(args)...);
	}

	template <typename T = Derived>
	static std::optional<bool> flush_if_supported(T &, Args &&...) {
		return std::nullopt;
	}

public:
	void flush_fully(Args &&...args);

protected:
	template <typename NewDerived, typename... NewArgs>
	constexpr auto _const recast() noexcept { return reinterpret_cast<Flushable<NewDerived, NewArgs...> &>(*this); }

};


class Source : public Readable<Source> {

public:
	using BufferPointer = ::BufferPointer;

public:
	virtual ~Source() = default;

public:
	_nodiscard virtual ssize_t read(void *buf, size_t n) = 0;
	_nodiscard virtual ssize_t read(std::span<const BufferPointer> bufs) { return this->Readable::read(bufs); }
	using Readable::read;

};

extern template class Readable<Source>;


class Sink : public Writable<Sink>, public Flushable<Sink> {

public:
	using BufferPointer = ::ConstBufferPointer;

public:
	virtual ~Sink() = default;

public:
	_nodiscard virtual size_t write(const void *buf, size_t n) = 0;
	_nodiscard virtual size_t write(std::span<const BufferPointer> bufs) { return this->Writable::write(bufs); }
	using Writable::write;

	virtual bool flush() { return true; }

};

extern template class Writable<Sink>;
extern template class Flushable<Sink>;


template <typename Input>
class InputSource : public Input, public Source {

public:
	using Input::Input;

public:
	_nodiscard ssize_t read(void *buf, size_t n) override { return this->unwrap().read(buf, n); }
	_nodiscard ssize_t read(std::span<const BufferPointer> bufs) override { return this->unwrap().read(bufs); }

	using Source::read;
	using Source::read_fully;

private:
	std::add_lvalue_reference_t<std::unwrap_reference_t<Input>> unwrap() { return *this; }

};

template <typename Input>
InputSource(Input &) -> InputSource<std::reference_wrapper<Input>>;

extern template class InputSource<std::reference_wrapper<Source>>;


template <typename Output>
class OutputSink : public Output, public Sink {

public:
	using Output::Output;

public:
	_nodiscard size_t write(const void *buf, size_t n) override { return this->unwrap().write(buf, n); }
	_nodiscard size_t write(std::span<const BufferPointer> bufs) override { return this->unwrap().write(bufs); }
	bool flush() override { return Flushable<std::unwrap_reference_t<Output>>::flush_if_supported(this->unwrap()).value_or(true); }

	using Sink::write;
	using Sink::write_fully;
	using Sink::flush_fully;

private:
	std::add_lvalue_reference_t<std::unwrap_reference_t<Output>> unwrap() { return *this; }

};

template <typename Output>
OutputSink(Output &) -> OutputSink<std::reference_wrapper<Output>>;

extern template class OutputSink<std::reference_wrapper<Sink>>;


template <typename Input = Source &>
class LimitedReadable : public Readable<LimitedReadable<Input>> {

public:
	Input input;
	uintmax_t remaining;

public:
	constexpr LimitedReadable(Input &&input, uintmax_t remaining) noexcept : input(std::forward<Input>(input)), remaining(remaining) { }

public:
	_nodiscard ssize_t read(void *buf, size_t n) {
		if (_unlikely(n > remaining)) {
			if (remaining == 0) {
				return -1;
			}
			n = saturate<size_t>(remaining);
		}
		if (_unlikely(n == 0)) {
			return 0;
		}
		ssize_t r = input.read(buf, n);
		if (r > 0) {
			remaining -= r;
		}
		return r;
	}

	using Readable<LimitedReadable<Input>>::read;

};

using LimitedSource = InputSource<LimitedReadable<>>;
extern template class InputSource<LimitedReadable<>>;


template <typename Output = Sink &>
class LimitedWritable : public Writable<LimitedWritable<Output>> {

public:
	Output output;
	uintmax_t remaining;

public:
	constexpr LimitedWritable(Output &&output, uintmax_t remaining) noexcept : output(std::forward<Output>(output)), remaining(remaining) { }

public:
	_nodiscard size_t write(const void *buf, size_t n) {
		if (_unlikely(n > remaining)) {
			n = saturate<size_t>(remaining);
		}
		if (_unlikely(n == 0)) {
			return 0;
		}
		size_t w = output.write(buf, n);
		remaining -= w;
		return w;
	}

	using Writable<LimitedWritable<Output>>::write;

};

using LimitedSink = OutputSink<LimitedWritable<>>;
extern template class OutputSink<LimitedWritable<>>;


_nodiscard ssize_t read(void *dst, ConstBufferView &src, size_t n) noexcept;
_nodiscard size_t write(BufferView &dst, const void *src, size_t n) noexcept;


class MemorySource : public ConstBufferView, public Source {

public:
	constexpr MemorySource(const void *buf, size_t n) noexcept : ConstBufferView(static_cast<const std::byte *>(buf), static_cast<const std::byte *>(buf), static_cast<const std::byte *>(buf) + n, static_cast<const std::byte *>(buf) + n) { }

	template <typename R>
	constexpr explicit MemorySource(R &&range) noexcept : MemorySource(std::data(std::forward<R>(range)), std::size(std::forward<R>(range)) * sizeof(typename std::remove_reference_t<R>::value_type)) { }

public:
	_nodiscard ssize_t read(void *buf, size_t n) override { return ::read(buf, *this, n); }

};


class MemorySink : public BufferView, public Sink {

public:
	constexpr MemorySink(void *buf, size_t n) noexcept : BufferView(static_cast<std::byte *>(buf), n) { }

public:
	_nodiscard size_t write(const void *buf, size_t n) override { return ::write(*this, buf, n); }

};


class StringSink : public Sink {

private:
	std::string &string;

public:
	constexpr explicit StringSink(std::string &string) noexcept : string(string) { }

public:
	_nodiscard size_t write(const void *buf, size_t n) override { return string.append(static_cast<const char *>(buf), n), n; }

};


class BufferViewSource : public Source {

private:
	ConstBufferView &buffer;

public:
	constexpr explicit BufferViewSource(ConstBufferView &buffer) noexcept : buffer(buffer) { }

public:
	_nodiscard ssize_t read(void *buf, size_t n) override { return ::read(buf, buffer, n); };

};


class BufferViewSink : public Sink {

private:
	BufferView &buffer;

public:
	constexpr explicit BufferViewSink(BufferView &buffer) noexcept : buffer(buffer) { }

public:
	_nodiscard size_t write(const void *buf, size_t n) override { return ::write(buffer, buf, n); }

};


class DynamicBufferSink : public DynamicBuffer, public Sink {

private:
	DynamicBuffer &buffer;

public:
	constexpr explicit DynamicBufferSink(DynamicBuffer &buffer) noexcept : buffer(buffer) { }

public:
	_nodiscard size_t write(const void *buf, size_t n) override { return buffer.append(buf, n), n; }

};


class BufferedSource : public Source, protected DynamicBuffer {

protected:
	Source &source;

public:
	explicit BufferedSource(Source &source) : BufferedSource(source, 8192) { }

	template <typename... Args>
	explicit BufferedSource(Source &source, Args &&...args) : DynamicBuffer(std::forward<Args>(args)...), source(source) { }

public:
	_nodiscard ssize_t read(void *buf, size_t n) override;

};


class BufferedSink : public Sink, protected DynamicBuffer {

protected:
	Sink &sink;

public:
	explicit BufferedSink(Sink &sink) : BufferedSink(sink, 8192) { }

	template <typename... Args>
	explicit BufferedSink(Sink &sink, Args &&...args) : DynamicBuffer(std::forward<Args>(args)...), sink(sink) { }

public:
	_nodiscard size_t write(const void *buf, size_t n) override;
	bool flush() override;

};


template <typename Input = Source &>
class DelimitedReadable : public Readable<DelimitedReadable<Input>> {

public:
	Input input;

private:
	const std::string_view delimiter;
	std::string_view::const_iterator delim_itr;

public:
	constexpr DelimitedReadable(Input &&input, std::string_view delimiter) noexcept : input(std::forward<Input>(input)), delimiter(delimiter), delim_itr(delimiter.begin()) { }

public:
	void reset() noexcept { delim_itr = delimiter.begin(); }

	_nodiscard ssize_t read(void *buf, size_t n) {
		ssize_t r = 0;
		while (n > 0) {
			std::ptrdiff_t d;
			if ((d = delimiter.end() - delim_itr) <= 0) {
				return r == 0 ? -1 : r;
			}
			ssize_t s;
			if ((s = input.read(buf, std::min(static_cast<size_t>(d), n))) <= 0) {
				return r == 0 ? s : r;
			}
			for (ssize_t i = 0; i < s; ++i) {
				char c = static_cast<char *>(buf)[i];
				delim_itr = c == *delim_itr ? delim_itr + 1 : c == *delimiter.begin() ? delimiter.begin() + 1 : delimiter.begin();
			}
			buf = static_cast<char *>(buf) + s, n -= s, r += s;
		}
		return r;
	}

	using Readable<DelimitedReadable<Input>>::read;

};

using DelimitedSource = InputSource<DelimitedReadable<>>;
extern template class InputSource<DelimitedReadable<>>;


class Tap : public Source {

private:
	Source &source;
	Sink &sink;

public:
	Tap(Source &source, Sink &sink) noexcept : source(source), sink(sink) { }

public:
	_nodiscard ssize_t read(void *buf, size_t n) override;

};


template <size_t N>
class Tee : public Sink {

private:
	const std::array<Sink *, N> sinks;

public:
	explicit Tee(const std::array<Sink *, N> &sinks) noexcept : sinks(sinks) { }

	template <typename... Args>
	explicit Tee(Args&&... args) noexcept : sinks({ std::forward<Args>(args)... }) { }

public:
	_nodiscard size_t write(const void *buf, size_t n) override {
		for (auto sink : sinks) {
			sink->write_fully(buf, n);
		}
		return n;
	}

	bool flush() override {
		bool ret = true;
		for (auto sink : sinks) {
			ret &= sink->flush();
		}
		return ret;
	}

};


class StreamBufSourceSink : public Source, public Sink {

private:
	std::streambuf &sb;

public:
	explicit StreamBufSourceSink(std::streambuf &sb) noexcept : sb(sb) { }

public:
	_nodiscard ssize_t read(void *buf, size_t n) override;

	_nodiscard size_t write(const void *buf, size_t n) override;
	bool flush() override { return sb.pubsync() == 0; }

};


class SourceBuf : public virtual std::streambuf {

protected:
	Source &source;

private:
	char_type gbuf;

public:
	explicit SourceBuf(Source &source) noexcept;
	SourceBuf(const SourceBuf &) = delete;
	SourceBuf & operator=(const SourceBuf &) = delete;
	SourceBuf(SourceBuf &&) = default;
	SourceBuf & operator=(SourceBuf &&) = default;

protected:
	std::streambuf * setbuf(char_type s[], std::streamsize n) override;
	int_type underflow() override;
	std::streamsize xsgetn(char_type s[], std::streamsize n) override;

};


class SinkBuf : public virtual std::streambuf {

protected:
	Sink &sink;

public:
	explicit SinkBuf(Sink &sink) noexcept : sink(sink) { }
	SinkBuf(const SinkBuf &) = delete;
	SinkBuf & operator=(const SinkBuf &) = delete;
	SinkBuf(SinkBuf &&) = default;
	SinkBuf & operator=(SinkBuf &&) = default;

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
	explicit SourceSinkBuf(T &source_sink) noexcept : SourceBuf(source_sink), SinkBuf(source_sink) { }

	SourceSinkBuf(Source &source, Sink &sink) noexcept : SourceBuf(source), SinkBuf(sink) { }

protected:
	std::streambuf * setbuf(char_type s[], std::streamsize n) override;

};


class MemoryBuf : public std::streambuf {

public:
	MemoryBuf(const void *buf, size_t n);

protected:
	MemoryBuf * setbuf(char s[], std::streamsize n) override;

};
