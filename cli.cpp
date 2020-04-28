#include "cli.h"

#include <cstring>


namespace cli {


static std::string make_what(std::string_view msg, const AbstractOption &opt) {
	std::string what;
	if (opt.long_form) {
		what.reserve(std::strlen(opt.long_form) + 4 + msg.size());
		what.append("--").append(opt.long_form).append(": ").append(msg);
	}
	else {
		what.reserve(4 + msg.size());
		what.push_back('-');
		what.push_back(opt.short_form);
		what.append(": ").append(msg);
	}
	return what;
}

OptionException::OptionException(std::string_view msg, const AbstractOption &opt) : runtime_error(make_what(msg, opt)) {
}

int parse(int argc, char *argv[], std::span<AbstractOption * const> opts) {
	char **head = argv, **tail = argv, **end = argv + argc;
	while (head < end) {
		if ((*head)[0] == '-') {
			if ((*head)[1] == '-') {
				if ((*head)[2] == '\0') {
					while (++head < end) {
						*tail++ = *head;
					}
					break;
				}
				char *opt = *head + 2, *eq = std::strchr(opt, '=');
				if (eq) {
					*eq = '\0';
					*head = eq + 1;
				}
				else {
					++head;
				}
				for (auto opt_ptr : opts) {
					if (opt_ptr->long_form) {
						if (std::strcmp(opt_ptr->long_form, opt) == 0) {
							opt_ptr->parse(opt_ptr->takes_arg() > 0 || eq ? *head++ : nullptr);
							goto next_arg;
						}
					}
				}
				throw OptionException(std::string("--") + opt + ": unrecognized option");
			}
			else if ((*head)[1] != '\0') {
				++*head;
				do {
					char opt = *(*head)++;
					for (auto opt_ptr : opts) {
						if (opt_ptr->short_form == opt) {
							if (**head == '\0') {
								++head;
								opt_ptr->parse(opt_ptr->takes_arg() > 0 ? *head++ : nullptr);
								goto next_arg;
							}
							if (opt_ptr->takes_arg()) {
								opt_ptr->parse(*head++);
								goto next_arg;
							}
							opt_ptr->parse(nullptr);
							goto next_opt;
						}
					}
					throw OptionException(std::string("-") + opt + ": unrecognized option");
				next_opt:;
				} while (**head != '\0');
				++head;
				goto next_arg;
			}
		}
		*tail++ = *head++;
	next_arg:;
	}
	std::memset(tail, 0, (end - tail) * sizeof(char *));
	return static_cast<int>(tail - argv);
}


} // namespace cli
