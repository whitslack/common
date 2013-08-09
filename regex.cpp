#include "regex.h"


const char * RegexCategory::name() const noexcept {
	return "regex";
}

std::string RegexCategory::message(int condition) const {
	std::string msg;
	msg.resize(::regerror(condition, regex, nullptr, 0));
	msg.resize(::regerror(condition, regex, &msg.front(), msg.size()) - 1);
	return msg;
}


Regex::Regex(const char regex[], int cflags) {
	int error;
	if ((error = ::regcomp(&this->regex, regex, cflags)) != 0) {
		throw RegexError(error, *this, "regcomp");
	}
}

bool Regex::exec(const char string[], size_t nmatch, regmatch_t pmatch[], int eflags) const {
	int error;
	if ((error = ::regexec(&regex, string, nmatch, pmatch, eflags)) != 0) {
		if (error == REG_NOMATCH) {
			return false;
		}
		throw RegexError(error, *this, "regexec");
	}
	return true;
}
