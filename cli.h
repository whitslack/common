#include <chrono>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <vector>

#include "compiler.h"
#include "narrow.h"


namespace cli {


template <typename Func, typename... Args>
static inline std::result_of_t<Func(const char *, char **, Args...)> strto(Func strto, const char str[], Args &&...args) {
	char *end = nullptr;
	auto ret = strto(str, &end, std::forward<Args>(args)...);
	if (_unlikely(!end || end == str || *end)) {
		throw std::invalid_argument("expected a number");
	}
	return ret;
}

static inline void convert(short &out, const char str[]) {
	out = narrow_check<short>(strto(std::strtol, str, 0));
}

static inline void convert(int &out, const char str[]) {
	out = narrow_check<int>(strto(std::strtol, str, 0));
}

static inline void convert(long &out, const char str[]) {
	out = strto(std::strtol, str, 0);
}

static inline void convert(long long &out, const char str[]) {
	out = strto(std::strtoll, str, 0);
}

static inline void convert(unsigned short &out, const char str[]) {
	out = narrow_check<unsigned short>(strto(std::strtoul, str, 0));
}

static inline void convert(unsigned &out, const char str[]) {
	out = narrow_check<unsigned>(strto(std::strtoul, str, 0));
}

static inline void convert(unsigned long &out, const char str[]) {
	out = strto(std::strtoul, str, 0);
}

static inline void convert(unsigned long long &out, const char str[]) {
	out = strto(std::strtoull, str, 0);
}

static inline void convert(float &out, const char str[]) {
	out = strto(std::strtof, str);
}

static inline void convert(double &out, const char str[]) {
	out = strto(std::strtod, str);
}

static inline void convert(long double &out, const char str[]) {
	out = strto(std::strtold, str);
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
	friend int parse(int, char *[], AbstractOption * const [], size_t);

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

	const argument_type & _pure operator [] (size_t i) const noexcept {
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


int parse(int argc, char *argv[], AbstractOption * const opts[], size_t n_opts);

template <size_t N>
static inline int parse(int argc, char *argv[], AbstractOption * const (&opts)[N]) {
	return parse(argc, argv, opts, N);
}

static inline int parse(int argc, char *argv[], std::initializer_list<AbstractOption *> opts) {
	return parse(argc, argv, opts.begin(), opts.size());
}


} // namespace cli
