#include <chrono>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <vector>

#include "charconv.h"
#include "compiler.h"
#include "narrow.h"
#include "span.h"


namespace cli {


template <typename T>
static inline std::enable_if_t<std::is_integral_v<T>> convert(T &out, std::string_view sv) {
	int base = 10;
	if (std::is_unsigned_v<T> && sv.size() >= 2 && sv[0] == '0' && (sv[1] | 0x20) == 'x') {
		sv.remove_prefix(2);
		base = 16;
	}
	auto result = std::from_chars(sv.begin(), sv.end(), out, base);
	if (_likely(result.ec == std::errc { })) {
		if (_likely(result.ptr == sv.end())) {
			return;
		}
	}
	else if (result.ec == std::errc::result_out_of_range) {
		throw std::range_error("out of range");
	}
	throw std::invalid_argument(std::is_unsigned_v<T> ? "expected an unsigned integer" : "expected an integer");
}

template <typename T>
static inline std::enable_if_t<std::is_floating_point_v<T>> convert(T &out, std::string_view sv) {
	auto result = std::from_chars(sv.begin(), sv.end(), out);
	if (_likely(result.ec == std::errc { })) {
		if (_likely(result.ptr == sv.end())) {
			return;
		}
	}
	else if (result.ec == std::errc::result_out_of_range) {
		throw std::range_error("out of range");
	}
	throw std::invalid_argument("expected a number");
}

template<typename Arg>
static inline auto convert(Arg &out, const char str[]) noexcept(noexcept(out = str)) -> std::void_t<decltype(out = str)> {
	out = str;
}

template<typename Rep, typename Period>
static inline auto convert(std::chrono::duration<Rep, Period> &out, const char str[]) {
	Rep rep;
	convert(rep, str);
	out = std::chrono::duration<Rep, Period> { rep };
}


class AbstractOption {
	friend int parse(int, char *[], std::span<AbstractOption * const>);

public:
	const char * const long_form;
	const char short_form;

public:
	explicit AbstractOption(const char long_form[], char short_form = '\0') noexcept : long_form(long_form), short_form(short_form) { }

	virtual ~AbstractOption() = default;

protected:
	virtual int takes_arg() const noexcept _pure = 0;

	virtual void parse(char arg[]) = 0;

};


template <typename Arg>
class WithArgument {

public:
	typedef Arg argument_type;

public:
	std::vector<argument_type> args;

public:
	explicit _pure operator bool () const noexcept {
		return !args.empty();
	}

	const argument_type & _pure value() const noexcept {
		return args.back();
	}

	size_t _pure count() const noexcept {
		return args.size();
	}

	const argument_type & _pure operator[](size_t i) const noexcept {
		return args[i];
	}

};


class OptionException : public std::runtime_error {

public:
	using std::runtime_error::runtime_error;

	OptionException(std::string_view msg, const AbstractOption &opt);

};


template <typename Arg = void>
class Option : public AbstractOption, public WithArgument<Arg> {

public:
	using AbstractOption::AbstractOption;

public:
	template <typename Def>
	Arg _pure value_or(Def &&def) const {
		return this->args.empty() ? Arg { std::forward<Def>(def) } : this->args.back();
	}

protected:
	int _pure takes_arg() const noexcept override { return 1; }

	void parse(char arg[]) override {
		if (_unlikely(!arg)) {
			throw OptionException("option requires an argument", *this);
		}
		try {
			Arg converted;
			convert(converted, arg); // unqualified call invokes argument-dependent lookup
			this->args.emplace_back(std::move(converted));
		}
		catch (const std::exception &e) {
			throw OptionException(e.what(), *this);
		}
	}

};


template <typename Arg>
class Option<std::optional<Arg>> : public AbstractOption, public WithArgument<std::optional<Arg>> {

public:
	using AbstractOption::AbstractOption;

public:
	template <typename Def>
	Arg _pure value_or(Def &&def) const {
		return this->args.empty() ? Arg { std::forward<Def>(def) } : this->args.back().value_or(std::forward<Def>(def));
	}

protected:
	int _pure takes_arg() const noexcept override { return -1; }

	void parse(char arg[]) override {
		if (!arg) {
			return this->args.emplace_back();
		}
		try {
			Arg converted;
			convert(converted, arg); // unqualified call invokes argument-dependent lookup
			return this->args.emplace_back(std::move(converted));
		}
		catch (const std::exception &e) {
			throw OptionException(e.what(), *this);
		}
	}

};


template <>
class Option<void> : public AbstractOption {

public:
	typedef void argument_type;

public:
	size_t occurrences = 0;

public:
	using AbstractOption::AbstractOption;

public:
	explicit _pure operator bool () const noexcept {
		return occurrences;
	}

	size_t _pure count() const noexcept {
		return occurrences;
	}

protected:
	int _pure takes_arg() const noexcept override { return 0; }

	void parse(char arg[]) override {
		if (_unlikely(arg)) {
			throw OptionException("option does not accept an argument", *this);
		}
		++occurrences;
	}

};


int parse(int argc, char *argv[], std::span<AbstractOption * const> opts);

static inline int parse(int argc, char *argv[], std::initializer_list<AbstractOption *> opts) {
	return parse(argc, argv, { opts });
}


} // namespace cli
