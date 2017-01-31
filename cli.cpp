#include "cli.h"

#include <cstring>


namespace cli {


static std::string make_what(const std::string &msg, const AbstractOption &opt) {
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

OptionException::OptionException(const std::string &msg, const AbstractOption &opt) : runtime_error(make_what(msg, opt)) {
}

int parse(int argc, char *argv[], AbstractOption * const opts[], size_t n_opts) {
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
				for (size_t i = 0; i < n_opts; ++i) {
					if (opts[i]->long_form) {
						if (std::strcmp(opts[i]->long_form, opt) == 0) {
							opts[i]->parse(opts[i]->takes_arg() > 0 || eq ? *head++ : nullptr);
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
					for (size_t i = 0; i < n_opts; ++i) {
						if (opts[i]->short_form == opt) {
							if (**head == '\0') {
								++head;
								opts[i]->parse(opts[i]->takes_arg() > 0 ? *head++ : nullptr);
								goto next_arg;
							}
							if (opts[i]->takes_arg()) {
								opts[i]->parse(*head++);
								goto next_arg;
							}
							opts[i]->parse(nullptr);
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
