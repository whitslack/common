#include <system_error>

#include <regex.h>

#include "compiler.h"


class Regex {

private:
	regex_t regex;

public:
	Regex(const char regex[], int cflags = REG_EXTENDED);
	~Regex() { ::regfree(&regex); }
	_pure operator regex_t * () noexcept { return &regex; }
	_pure operator const regex_t * () const noexcept { return &regex; }

	bool exec(const char string[], size_t nmatch, regmatch_t pmatch[], int eflags = 0) const;

private:
	Regex(const Regex &) = delete;
	Regex & operator = (const Regex &) = delete;

};


class RegexCategory : public std::error_category {

private:
	const regex_t *regex;

public:
	RegexCategory(const Regex &regex) noexcept : regex(regex) { }
	const char * name() const noexcept override _const;
	std::string message(int condition) const override _pure;

};


class RegexError : public std::system_error {

public:
	template <typename... Args>
	RegexError(int ev, const Regex &regex, Args&&... args) : std::system_error(ev, RegexCategory(regex), std::forward<Args>(args)...) { }

};
