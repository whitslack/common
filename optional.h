#include <stdexcept>


static constexpr struct nullopt_t { } nullopt;

static constexpr struct in_place_t { } in_place;

class bad_optional_access : public std::logic_error {
public:
	using std::logic_error::logic_error;
};


template <typename T>
class optional {

public:
	typedef T value_type;

private:
	T *ptr;
	typename std::aligned_storage<sizeof(T), alignof(T)>::type data;

public:
	constexpr optional() noexcept : ptr() { }

	constexpr optional(nullopt_t) noexcept : ptr() { }

	constexpr optional(const optional &other) noexcept(std::is_nothrow_copy_constructible<T>::value) : ptr(other.ptr ? new (&data) T(*other.ptr) : nullptr) { }

	constexpr optional(optional &&other) noexcept(std::is_nothrow_move_constructible<T>::value) : ptr(other.ptr ? new (&data) T(std::move(*other.ptr)) : nullptr) { }

	constexpr optional(const T &value) noexcept(std::is_nothrow_copy_constructible<T>::value) : ptr(new (&data) T(value)) { }

	constexpr optional(T &&value) noexcept(std::is_nothrow_move_constructible<T>::value) : ptr(new (&data) T(std::move(value))) { }

	template <typename... Args>
	constexpr explicit optional(in_place_t, Args&&... args) noexcept(std::is_nothrow_constructible<T, Args&&...>::value) : ptr(new (&data) T(std::forward<Args>(args)...)) { }

	template <typename U, typename... Args>
	constexpr explicit optional(typename std::enable_if<std::is_constructible<T, std::initializer_list<U> &, Args&&...>::value, in_place_t>::type, std::initializer_list<U> ilist, Args&&... args) noexcept(std::is_nothrow_constructible<T, std::initializer_list<U> &, Args&&...>::value) : ptr(new (&data) T(ilist, std::forward<Args>(args)...)) { }
	
	~optional() {
		if (ptr) {
			ptr->~T(), ptr = nullptr;
		}
	}

public:
	optional & operator = (nullopt_t) noexcept {
		if (ptr) {
			ptr->~T(), ptr = nullptr;
		}
		return *this;
	}

	optional & operator = (const optional &other) noexcept(std::is_nothrow_copy_assignable<T>::value && std::is_nothrow_copy_constructible<T>::value) {
		if (ptr) {
			if (other.ptr) {
				*ptr = *other.ptr;
			}
			else {
				ptr->~T(), ptr = nullptr;
			}
		}
		else if (other.ptr) {
			ptr = new (&data) T(*other.ptr);
		}
		return *this;
	}

	optional & operator = (optional &&other) noexcept(std::is_nothrow_move_assignable<T>::value && std::is_nothrow_move_constructible<T>::value) {
		if (ptr) {
			if (other.ptr) {
				*ptr = std::move(*other.ptr);
			}
			else {
				ptr->~T(), ptr = nullptr;
			}
		}
		else if (other.ptr) {
			ptr = new (&data) T(std::move(*other.ptr));
		}
		return *this;
	}

	template <typename U>
	typename std::enable_if<std::is_assignable<T, U &&>::value && std::is_constructible<T, U &&>::value, optional>::type & operator = (U &&value) noexcept(std::is_nothrow_assignable<T, U &&>::value && std::is_nothrow_constructible<T, U &&>::value) {
		if (ptr) {
			*ptr = std::forward<U>(value);
		}
		else {
			ptr = new (&data) T(std::forward<U>(value));
		}
		return *this;
	}

	constexpr const T * operator -> () const noexcept {
		return ptr;
	}

	T * operator -> () noexcept {
		return ptr;
	}

	constexpr const T & operator * () const noexcept {
		return *ptr;
	}

	T & operator * () noexcept {
		return *ptr;
	}

	constexpr explicit operator bool () const noexcept {
		return ptr;
	}

	constexpr const T & value() const {
		if (!ptr) {
			throw bad_optional_access("optional object is disengaged");
		}
		return *ptr;
	}

	T & value() {
		if (!ptr) {
			throw bad_optional_access("optional object is disengaged");
		}
		return *ptr;
	}

	template <typename U>
	constexpr T value_or(U &&value) const & noexcept {
		return ptr ? *ptr : std::forward<U>(value);
	}

	template <typename U>
	T value_or(U &&value) && noexcept {
		return ptr ? *ptr : std::forward<U>(value);
	}

	void swap(optional &other) noexcept(std::is_nothrow_move_constructible<T>::value && noexcept(swap(std::declval<T &>(), std::declval<T &>()))) {
		if (ptr) {
			if (other.ptr) {
				using std::swap;
				swap(**this, *other);
			}
			else {
				other.ptr = new (&other.data) T(std::move(*ptr));
				ptr->~T(), ptr = nullptr;
			}
		}
		else if (other.ptr) {
			ptr = new (&data) T(std::move(*other.ptr));
			other.ptr->~T(), other.ptr = nullptr;
		}
	}

	template <typename... Args>
	void emplace(Args&&... args) noexcept(std::is_nothrow_constructible<T, Args&&...>::value) {
		if (ptr) {
			ptr->~T(), ptr = nullptr;
		}
		ptr = new (&data) T(std::forward<Args>(args)...);
	}

	template <typename U, typename... Args>
	void emplace(std::initializer_list<U> ilist, Args&&... args) noexcept(std::is_nothrow_constructible<T, std::initializer_list<U> &, Args&&...>::value) {
		if (ptr) {
			ptr->~T(), ptr = nullptr;
		}
		ptr = new (&data) T(ilist, std::forward<Args>(args)...);
	}

};


template <typename T>
constexpr bool operator == (const optional<T> &lhs, const optional<T> &rhs) noexcept(std::declval<const T &>() == std::declval<const T &>()) {
	return lhs ? rhs && *lhs == *rhs : !rhs;
}

template <typename T>
constexpr bool operator != (const optional<T> &lhs, const optional<T> &rhs) noexcept(std::declval<const T &>() != std::declval<const T &>()) {
	return lhs ? !rhs || *lhs != *rhs : rhs;
}

template <typename T>
constexpr bool operator < (const optional<T> &lhs, const optional<T> &rhs) noexcept(std::declval<const T &>() < std::declval<const T &>()) {
	return rhs && (!lhs || *lhs < *rhs);
}

template <typename T>
constexpr bool operator >= (const optional<T> &lhs, const optional<T> &rhs) noexcept(std::declval<const T &>() >= std::declval<const T &>()) {
	return !rhs || lhs && *lhs >= *rhs;
}

template <typename T>
constexpr bool operator > (const optional<T> &lhs, const optional<T> &rhs) noexcept(std::declval<const T &>() > std::declval<const T &>()) {
	return lhs && (!rhs || *lhs > *rhs);
}

template <typename T>
constexpr bool operator <= (const optional<T> &lhs, const optional<T> &rhs) noexcept(std::declval<const T &>() <= std::declval<const T &>()) {
	return !lhs || rhs && *lhs <= *rhs;
}


template <typename T>
constexpr bool operator == (const optional<T> &opt, nullopt_t) noexcept {
	return !opt;
}

template <typename T>
constexpr bool operator != (const optional<T> &opt, nullopt_t) noexcept {
	return opt;
}

template <typename T>
constexpr bool operator == (nullopt_t, const optional<T> &opt) noexcept {
	return !opt;
}

template <typename T>
constexpr bool operator != (nullopt_t, const optional<T> &opt) noexcept {
	return opt;
}

template <typename T>
constexpr bool operator < (const optional<T> &opt, nullopt_t) noexcept {
	return false;
}

template <typename T>
constexpr bool operator >= (const optional<T> &opt, nullopt_t) noexcept {
	return true;
}

template <typename T>
constexpr bool operator < (nullopt_t, const optional<T> &opt) noexcept {
	return opt;
}

template <typename T>
constexpr bool operator >= (nullopt_t, const optional<T> &opt) noexcept {
	return !opt;
}

template <typename T>
constexpr bool operator > (const optional<T> &opt, nullopt_t) noexcept {
	return opt;
}

template <typename T>
constexpr bool operator <= (const optional<T> &opt, nullopt_t) noexcept {
	return !opt;
}

template <typename T>
constexpr bool operator > (nullopt_t, const optional<T> &opt) noexcept {
	return false;
}

template <typename T>
constexpr bool operator <= (nullopt_t, const optional<T> &opt) noexcept {
	return true;
}


template <typename T>
constexpr bool operator == (const optional<T> &opt, const T& v) noexcept(std::declval<const T &>() == std::declval<const T &>()) {
	return opt && *opt == v;
}

template <typename T>
constexpr bool operator != (const optional<T> &opt, const T& v) noexcept(std::declval<const T &>() != std::declval<const T &>()) {
	return !opt || *opt != v;
}

template <typename T>
constexpr bool operator == (const T& v, const optional<T> &opt) noexcept(std::declval<const T &>() == std::declval<const T &>()) {
	return opt && v == *opt;
}

template <typename T>
constexpr bool operator != (const T& v, const optional<T> &opt) noexcept(std::declval<const T &>() != std::declval<const T &>()) {
	return !opt || v != *opt;
}

template <typename T>
constexpr bool operator < (const optional<T> &opt, const T& v) noexcept(std::declval<const T &>() < std::declval<const T &>()) {
	return !opt || *opt < v;
}

template <typename T>
constexpr bool operator >= (const optional<T> &opt, const T& v) noexcept(std::declval<const T &>() >= std::declval<const T &>()) {
	return opt && *opt >= v;
}

template <typename T>
constexpr bool operator < (const T& v, const optional<T> &opt) noexcept(std::declval<const T &>() < std::declval<const T &>()) {
	return opt && v < *opt;
}

template <typename T>
constexpr bool operator >= (const T& v, const optional<T> &opt) noexcept(std::declval<const T &>() >= std::declval<const T &>()) {
	return !opt || v >= *opt;
}

template <typename T>
constexpr bool operator > (const optional<T> &opt, const T& v) noexcept(std::declval<const T &>() > std::declval<const T &>()) {
	return opt && *opt > v;
}

template <typename T>
constexpr bool operator <= (const optional<T> &opt, const T& v) noexcept(std::declval<const T &>() <= std::declval<const T &>()) {
	return !opt || *opt <= v;
}

template <typename T>
constexpr bool operator > (const T& v, const optional<T> &opt) noexcept(std::declval<const T &>() > std::declval<const T &>()) {
	return !opt || v > *opt;
}

template <typename T>
constexpr bool operator <= (const T& v, const optional<T> &opt) noexcept(std::declval<const T &>() <= std::declval<const T &>()) {
	return opt && v <= *opt;
}


template <typename T>
constexpr optional<typename std::decay<T>::type> make_optional(T &&value) noexcept(std::is_nothrow_constructible<optional<typename std::decay<T>::type>, T &&>::value) {
	return optional<typename std::decay<T>::type>(std::forward<T>(value));
}

template <typename T>
void swap(optional<T> &lhs, optional<T> &rhs) noexcept(noexcept(std::declval<optional<T> &>().swap(std::declval<optional<T> &>()))) {
	lhs.swap(rhs);
}

namespace std {
template <typename T>
struct hash<optional<T>> {
	constexpr size_t operator () (const optional<T> &opt) const noexcept(noexcept(std::hash<T>()(std::declval<const T &>()))) {
		return opt ? std::hash<T>()(*opt) : 0;
	}
};
}
