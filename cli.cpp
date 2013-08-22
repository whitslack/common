#include "cli.h"

#include <cstring>

namespace cli {


static std::string make_what(const char msg[], const Option &opt) {
	std::string what(msg);
	if (opt.long_form) {
		(what += ": --") += opt.long_form;
	}
	else {
		(what += ": -") += opt.short_form;
	}
	return what;
}

OptionException::OptionException(const char msg[], const Option &opt) : runtime_error(make_what(msg, opt)) {
}


bool BooleanOption::parse(char []) {
	value = true;
	return false;
}


bool StringOption::parse(char value[]) {
	if (value) {
		this->value = value;
		return true;
	}
	throw OptionException("expected argument for option", *this);
}


bool StringVectorOption::parse(char value[]) {
	if (value) {
		if (delim == '\0') {
			values.push_back(value);
			return true;
		}
		for (;;) {
			char *d = std::strchr(value, delim);
			if (!d) {
				values.push_back(value);
				return true;
			}
			*d = '\0';
			values.push_back(value);
			value = d + 1;
		}
	}
	throw OptionException("expected argument for option", *this);
}


int parse(int argc, char *argv[], Option * const opts[], size_t numopts) {
	int unused = 0;
	for (int i = 0; i < argc; ++i) {
		char *arg = argv[i];
		if (arg[0] == '-') {
			if (arg[1] == '-') {
				if (arg[2] == '\0') {
					while (++i < argc) {
						argv[unused++] = argv[i];
					}
					return unused;
				}
				else {
					arg += 2;
					size_t optlen = std::strcspn(arg, "=");
					for (size_t j = 0; j < numopts; ++j) {
						const char *long_form = opts[j]->long_form;
						if (long_form && std::strncmp(long_form, arg, optlen) == 0 && long_form[optlen] == '\0') {
							if (arg[optlen] == '\0') {
								if (i + 1 < argc) {
									if (opts[j]->parse(argv[i + 1])) {
										++i;
									}
								}
								else if (opts[j]->parse(nullptr)) {
									throw std::runtime_error("internal error while parsing command line");
								}
							}
							else if (!opts[j]->parse(arg + optlen + 1)) {
								throw OptionException("option takes no argument", *opts[j]);
							}
							goto next_arg;
						}
					}
					throw OptionException(std::string("unrecognized option: -") + (arg - 1));
				}
			}
			else if (arg[1] != '\0') {
				for (char *flag = arg + 1; *flag != '\0'; ++flag) {
					for (size_t j = 0; j < numopts; ++j) {
						if (opts[j]->short_form == *flag) {
							if (flag[1] == '\0') {
								if (i + 1 < argc) {
									if (opts[j]->parse(argv[i + 1])) {
										++i;
									}
								}
								else if (opts[j]->parse(nullptr)) {
									throw std::runtime_error("internal error while parsing command line");
								}
							}
							else if (!opts[j]->parse(flag + 1)) {
								goto next_flag;
							}
							goto next_arg;
						}
					}
					throw OptionException(std::string("unrecognized option: -") + *flag);
				next_flag:;
				}
				goto next_arg;
			}
		}
		argv[unused++] = argv[i];
	next_arg:;
	}
	return unused;
}


} // namespace cli
