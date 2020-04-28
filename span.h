#pragma once

#if __has_include(<span>)
#	include <span>
#else

#include <array>
#include <cstddef>
#include <iterator>
#include <limits>
#include <type_traits>


namespace polyfill {


inline constexpr std::size_t dynamic_extent = std::numeric_limits<std::size_t>::max();


template <typename T, std::size_t Extent>
class _span_base {

public:
	using element_type = T;
	using value_type = std::remove_cv_t<T>;
	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;
	using pointer = T*;
	using const_pointer = const T*;
	using reference = T&;
	using const_reference = const T&;
	using iterator = pointer;
	using reverse_iterator = std::reverse_iterator<iterator>;

public:
	static constexpr std::size_t extent = Extent;

};


template <typename T, std::size_t Extent = dynamic_extent>
class span : public _span_base<T, Extent> {

private:
	T *data_;

public:
	template <std::size_t N = 0, typename = std::enable_if_t<Extent == N>>
	constexpr span() noexcept : data_(nullptr) { }

	template <typename It, typename = std::enable_if_t<std::is_convertible_v<typename std::iterator_traits<It>::value_type(*)[], T(*)[]>>>
	constexpr span(It first, std::size_t) : data_(std::to_address(first)) { }

	template <typename It, typename End, typename = std::enable_if_t<std::is_same_v<typename std::iterator_traits<It>::value_type, T> && !std::is_convertible_v<End, std::size_t> && std::is_convertible_v<decltype(std::declval<End>() - std::declval<It>()), std::size_t>>>
	constexpr span(It first, End last) : data_(std::to_address(first)) { }

	template <std::size_t N, typename = std::enable_if_t<Extent == N>>
	constexpr span(T (&arr)[N]) noexcept : data_(std::data(arr)) { }

	template <typename U, std::size_t N, typename = std::enable_if_t<Extent == N && std::is_convertible_v<std::remove_pointer_t<decltype(std::data(std::declval<std::array<U, N> &>()))>(*)[], T(*)[]>>>
	constexpr span(std::array<U, N> &arr) noexcept : data_(std::data(arr)) { }

	template <typename U, std::size_t N, typename = std::enable_if_t<Extent == N && std::is_convertible_v<std::remove_pointer_t<decltype(std::data(std::declval<const std::array<U, N> &>()))>(*)[], T(*)[]>>>
	constexpr span(const std::array<U, N> &arr) noexcept : data_(std::data(arr)) { }

	template <typename U, std::size_t N, typename = std::enable_if_t<Extent == N && std::is_convertible_v<U(*)[], T(*)[]>>>
	constexpr span(const span<U, N> &s) noexcept : data_(s.data()) { }

public:
	constexpr T * begin() const noexcept { return this->data(); }
	constexpr T * end() const noexcept { return this->data() + this->size(); }
	constexpr std::reverse_iterator<T *> rbegin() const noexcept { return std::reverse_iterator<T *>(this->end()); }
	constexpr std::reverse_iterator<T *> rend() const noexcept { return std::reverse_iterator<T *>(this->begin()); }

	constexpr T & front() const noexcept { return *this->begin(); }
	constexpr T & back() const noexcept { return *this->rbegin(); }
	constexpr T & operator[](std::size_t idx) const noexcept { return this->data()[idx]; }
	constexpr T * data() const noexcept { return data_; }

	constexpr std::size_t size() const noexcept { return this->extent; }
	constexpr std::size_t size_bytes() const noexcept { return this->size() * sizeof(T); }
	constexpr bool empty() const noexcept { return this->size() == 0; }

	template <std::size_t Count>
	constexpr span<T, Count> first() const { return { this->data(), Count }; }
	constexpr span<T> first(std::size_t Count) const { return { this->data(), Count }; }

	template <std::size_t Count>
	constexpr span<T, Count> last() const { return { this->data() + (this->size() - Count), Count }; }
	constexpr span<T> last(std::size_t Count) const { return { this->data() + (this->size() - Count), Count }; }

	template <std::size_t Offset, std::size_t Count = dynamic_extent>
	constexpr span<T, Count> subspan() const { return { this->data() + Offset, Count == dynamic_extent ? this->size() - Offset : Count }; }
	constexpr span<T> subspan(std::size_t Offset, std::size_t Count = dynamic_extent) const { return { this->data() + Offset, Count == dynamic_extent ? this->size() - Offset : Count }; }

};


} // namespace polyfill


namespace {


template <typename T>
struct is_span : std::false_type { };

template <typename T, std::size_t Extent>
struct is_span<polyfill::span<T, Extent>> : std::true_type { };

template <typename T>
constexpr bool is_span_v = is_span<T>::value;


template <typename T>
struct is_std_array : std::false_type { };

template <typename T, std::size_t N>
struct is_std_array<std::array<T, N>> : std::true_type { };

template <typename T>
constexpr bool is_std_array_v = is_std_array<T>::value;


} // anonymous namespace


namespace polyfill {


template <typename T>
class span<T, dynamic_extent> : public _span_base<T, dynamic_extent> {

private:
	T *data_;
	std::size_t size_;

public:
	constexpr span() noexcept : data_(nullptr), size_(0) { }

	template <typename It, typename = std::enable_if_t<std::is_convertible_v<typename std::iterator_traits<It>::value_type(*)[], T(*)[]>>>
	constexpr span(It first, std::size_t count) : data_(std::to_address(first)), size_(count) { }

	template <typename It, typename End, typename = std::enable_if_t<std::is_same_v<typename std::iterator_traits<It>::value_type, T> && !std::is_convertible_v<End, std::size_t> && std::is_convertible_v<decltype(std::declval<End>() - std::declval<It>()), std::size_t>>>
	constexpr span(It first, End last) : data_(std::to_address(first)), size_(last - first) { }

	template <std::size_t N>
	constexpr span(T (&arr)[N]) noexcept : data_(std::data(arr)), size_(N) { }

	template <typename U, std::size_t N, typename = std::enable_if_t<std::is_convertible_v<std::remove_pointer_t<decltype(std::data(std::declval<std::array<U, N> &>()))>(*)[], T(*)[]>>>
	constexpr span(std::array<U, N> &arr) noexcept : data_(std::data(arr)), size_(N) { }

	template <typename U, std::size_t N, typename = std::enable_if_t<std::is_convertible_v<std::remove_pointer_t<decltype(std::data(std::declval<const std::array<U, N> &>()))>(*)[], T(*)[]>>>
	constexpr span(const std::array<U, N> &arr) noexcept : data_(std::data(arr)), size_(N) { }

	template <typename R, typename = std::enable_if_t<std::is_convertible_v<std::remove_pointer_t<decltype(std::data(std::declval<R &&>()))>(*)[], T(*)[]> && !is_span_v<std::remove_cvref_t<R>> && !is_std_array_v<std::remove_cvref_t<R>> && !std::is_array_v<std::remove_cvref_t<R>>>>
	constexpr span(R &&r) : data_(std::data(r)), size_(std::size(r)) { }

	template <typename U, std::size_t N, typename = std::enable_if_t<std::is_convertible_v<U(*)[], T(*)[]>>>
	constexpr span(const span<U, N> &s) noexcept : data_(s.data()), size_(s.size()) { }

public:
	constexpr T * begin() const noexcept { return this->data(); }
	constexpr T * end() const noexcept { return this->data() + this->size(); }
	constexpr std::reverse_iterator<T *> rbegin() const noexcept { return std::reverse_iterator<T *>(this->end()); }
	constexpr std::reverse_iterator<T *> rend() const noexcept { return std::reverse_iterator<T *>(this->begin()); }

	constexpr T & front() const noexcept { return *this->begin(); }
	constexpr T & back() const noexcept { return *this->rbegin(); }
	constexpr T & operator[](std::size_t idx) const noexcept { return this->data()[idx]; }
	constexpr T * data() const noexcept { return data_; }

	constexpr std::size_t size() const noexcept { return size_; }
	constexpr std::size_t size_bytes() const noexcept { return this->size() * sizeof(T); }
	constexpr bool empty() const noexcept { return this->size() == 0; }

	template <std::size_t Count>
	constexpr span<T, Count> first() const { return { this->data(), Count }; }
	constexpr span<T> first(std::size_t Count) const { return { this->data(), Count }; }

	template <std::size_t Count>
	constexpr span<T, Count> last() const { return { this->data() + (this->size() - Count), Count }; }
	constexpr span<T> last(std::size_t Count) const { return { this->data() + (this->size() - Count), Count }; }

	template <std::size_t Offset, std::size_t Count = dynamic_extent>
	constexpr span<T, Count> subspan() const { return { this->data() + Offset, Count == dynamic_extent ? this->size() - Offset : Count }; }
	constexpr span<T> subspan(std::size_t Offset, std::size_t Count = dynamic_extent) const { return { this->data() + Offset, Count == dynamic_extent ? this->size() - Offset : Count }; }

};


template <typename T, std::size_t N>
std::enable_if_t<N != dynamic_extent, span<const std::byte, N * sizeof(T)>> as_bytes(span<T, N> s) noexcept {
	return { reinterpret_cast<const std::byte *>(s.data()), s.size_bytes() };
}

template <typename T, std::size_t N>
std::enable_if_t<N == dynamic_extent, span<const std::byte, dynamic_extent>> as_bytes(span<T, N> s) noexcept {
	return { reinterpret_cast<const std::byte *>(s.data()), s.size_bytes() };
}

template <typename T, std::size_t N>
std::enable_if_t<!std::is_const_v<T> && N != dynamic_extent, span<std::byte, N * sizeof(T)>> as_writable_bytes(span<T, N> s) noexcept {
	return { reinterpret_cast<std::byte *>(s.data()), s.size_bytes() };
}

template <typename T, std::size_t N>
std::enable_if_t<!std::is_const_v<T> && N == dynamic_extent, span<std::byte, dynamic_extent>> as_writable_bytes(span<T, N> s) noexcept {
	return { reinterpret_cast<std::byte *>(s.data()), s.size_bytes() };
}


template <typename It, typename EndOrSize>
span(It, EndOrSize) -> span<typename std::iterator_traits<It>::value_type>;

template <typename T, std::size_t N>
span(T (&)[N]) -> span<T, N>;

template <typename T, std::size_t N>
span(std::array<T, N> &) -> span<T, N>;

template <typename T, std::size_t N>
span(const std::array<T, N> &) -> span<const T, N>;

template <typename R>
span(R &&) -> span<std::remove_pointer_t<decltype(std::data(std::declval<R &&>()))>>;


} // namespace polyfill


namespace std {
	using ::polyfill::dynamic_extent;
	using ::polyfill::span;
	using ::polyfill::as_bytes;
	using ::polyfill::as_writable_bytes;
}


#endif // !__has_include(<span>)
