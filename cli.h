#include <limits>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace CLI {

	class Option {
		friend int CLI::parse(int, char *[], Option * const [], size_t);

		public:
			const char * const long_form;
			const char short_form;

		public:
			virtual ~Option() { }

		protected:
			Option(const char long_form[], char short_form) : long_form(long_form), short_form(short_form) { }
			virtual bool parse(char value[]) = 0;

		private:
			Option(const Option &) = delete;
			Option & operator = (const Option &) = delete;

	};

	class OptionException : public std::runtime_error {

		public:
			OptionException(const char msg[], const Option &opt) : runtime_error(OptionException::make_what(msg, opt)) { }
			OptionException(const std::string &msg) : runtime_error(msg) { }

		private:
			static std::string make_what(const char msg[], const Option &opt);

	};

	class BooleanOption : public Option {

		public:
			bool value;

		public:
			explicit BooleanOption(const char long_form[], char short_form = '\0') : Option(long_form, short_form), value(false) { }

		protected:
			bool parse(char value[]) override;

	};

	template <typename N>
	static N strto(const std::string &str, size_t *pos = nullptr);

	template <>
	int strto<int>(const std::string &str, size_t *pos) {
		return std::stoi(str, pos);
	}

	template <>
	long strto<long>(const std::string &str, size_t *pos) {
		return std::stol(str, pos);
	}

	template <>
	long long strto<long long>(const std::string &str, size_t *pos) {
		return std::stoll(str, pos);
	}

	template <>
	unsigned long strto<unsigned long>(const std::string &str, size_t *pos) {
		return std::stoul(str, pos);
	}

	template <>
	unsigned long long strto<unsigned long long>(const std::string &str, size_t *pos) {
		return std::stoull(str, pos);
	}

	template <>
	float strto<float>(const std::string &str, size_t *pos) {
		return std::stof(str, pos);
	}

	template <>
	double strto<double>(const std::string &str, size_t *pos) {
		return std::stod(str, pos);
	}

	template <>
	long double strto<long double>(const std::string &str, size_t *pos) {
		return std::stold(str, pos);
	}

	template <typename T = long>
	class NumericOption : public Option {

		public:
			typedef T num_type;

		public:
			num_type value;

		private:
			const num_type min;
			const num_type max;

		public:
			explicit NumericOption(const char long_form[], char short_form = '\0', num_type default_value = num_type(), num_type min = std::numeric_limits<num_type>::min(), num_type max = std::numeric_limits<num_type>::max()) : Option(long_form, short_form), value(default_value), min(min), max(max) { }

		protected:
			bool parse(char value[]) override {
				if (value) {
					try {
						size_t pos;
						this->value = strto<num_type>(value, &pos);
						if (value[pos] == '\0' && this->value >= min && this->value <= max) {
							return true;
						}
					}
					catch (const std::range_error &) {
						// swallow
					}
				}
				std::ostringstream msg;
				msg << "expected " << (std::numeric_limits<num_type>::is_integer ? "integer" : "numeric") << " argument in range [" << min << ',' << max << "] for option";
				throw CLI::OptionException(msg.str().c_str(), *this);
			}

	};

	class StringOption : public Option {

		public:
			const char *value;

		public:
			explicit StringOption(const char long_form[], char short_form = '\0', const char default_value[] = nullptr) : Option(long_form, short_form), value(default_value) { }

		protected:
			bool parse(char value[]) override;

		private:
			StringOption(const StringOption &) = delete;
			StringOption & operator = (const StringOption &) = delete;

	};

	template <typename T>
	class NumericVectorOption : public Option {

		public:
			typedef T num_type;

		public:
			std::vector<num_type> values;

		private:
			const char delim;
			const num_type min;
			const num_type max;

		public:
			explicit NumericVectorOption(const char long_form[], char short_form = '\0', char delim = ',', num_type min = std::numeric_limits<num_type>::min(), num_type max = std::numeric_limits<num_type>::max()) : Option(long_form, short_form), delim(delim), min(min), max(max) { }

		protected:
			bool parse(char value[]) override {
				if (value) {
					try {
						const char *endptr;
						num_type temp;
						for (;;) {
							temp = strto<num_type>(value, &endptr);
							if (endptr == value || (*endptr != '\0' && *endptr != delim) || temp < min || temp > max) {
								break;
							}
							values.push_back(temp);
							if (*endptr == '\0') {
								return true;
							}
							value = endptr + 1;
						}
					}
					catch (const std::range_error &) {
						// swallow
					}
				}
				std::ostringstream msg;
				msg << "expected " << (std::numeric_limits<num_type>::is_integer ? "integer" : "numeric") << " argument in range [" << min << ',' << max << "] for option";
				throw CLI::OptionException(msg.str().c_str(), *this);
			}

	};

	class StringVectorOption : public Option {

		public:
			std::vector<char *> values;

		private:
			const char delim;

		public:
			explicit StringVectorOption(const char long_form[], char short_form = '\0', char delim = '\0') : Option(long_form, short_form), values(), delim(delim) { }

		protected:
			bool parse(char value[]) override;

	};

	template <size_t numopts>
	static inline int parse(int argc, char *argv[], Option * const (&opts)[numopts]) {
		return CLI::parse(argc, argv, opts, numopts);
	}
	int parse(int argc, char *argv[], Option * const opts[], size_t numopts);

}
