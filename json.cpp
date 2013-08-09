#include "json.h"

#include <cctype>
#include <cmath>
#include <iomanip>
#include <limits>
#include <sstream>
#include <stdexcept>

namespace json {

std::ostream & Object::format(std::ostream &os) const {
	os.put('{');
	auto it = map.begin();
	if (it != map.end()) {
		for (;;) {
			String(it->first).format(os).put(':') << it->second;
			if (++it == map.end()) {
				break;
			}
			os.put(',');
		}
	}
	return os.put('}');
}


std::ostream & Array::format(std::ostream &os) const {
	os.put('[');
	auto it = vector.begin();
	if (it != vector.end()) {
		for (;;) {
			os << *it;
			if (++it == vector.end()) {
				break;
			}
			os.put(',');
		}
	}
	return os.put(']');
}


std::ostream & Integer::format(std::ostream &os) const {
	auto flags = os.flags(std::ios_base::dec);
	return os << std::setw(0) << value << std::setiosflags(flags);
}


std::ostream & Real::format(std::ostream &os) const {
	if (!std::isfinite(value)) {
		return os << "null";
	}
	auto flags = os.flags(std::ios_base::dec);
	auto precision = os.precision(std::numeric_limits<double>::digits10);
	os << std::setw(0) << value << std::setiosflags(flags);
	os.precision(precision);
	return os;
}


std::ostream & String::format(std::ostream &os) const {
	static const char HEX[] = "0123456789ABCDEF";
	os.put('"');
	for (char c : string) {
		switch (c) {
			case 0x08: // backspace (U+0008)
				os.put('\\').put('b');
				break;
			case 0x09: // character tabulation (U+0009)
				os.put('\\').put('t');
				break;
			case 0x0A: // line feed (U+000A)
				os.put('\\').put('n');
				break;
			case 0x0C: // form feed (U+000C)
				os.put('\\').put('f');
				break;
			case 0x0D: // carriage return (U+000D)
				os.put('\\').put('r');
				break;
			case '"': // quotation mark (U+0022)
				os.put('\\').put('"');
				break;
			case '\\': // reverse solidus (U+005C)
				os.put('\\').put('\\');
				break;
			default:
				if (static_cast<unsigned char>(c) <= 0x1F) {
					os.put('\\').put('u').put('0').put('0').put(HEX[c >> 4]).put(HEX[c & (1 << 4) - 1]);
				}
				else {
					os.put(c);
				}
				break;
		}
	}
	return os.put('"');
}


std::ostream & Boolean::format(std::ostream &os) const {
	auto flags = os.flags(std::ios_base::boolalpha);
	return os << std::setw(0) << value << std::setiosflags(flags);
}


static unsigned int from_hex(std::istream &is) {
	static const int8_t UNHEX[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, -1, -1, -1, -1, -1, -1, -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 10, 11, 12, 13, 14, 15 };
	int c = is.peek();
	if (c < '0' || c > 'f' || (c = UNHEX[c - '0']) < 0) {
		throw std::ios_base::failure("invalid hex digit");
	}
	is.get();
	return c;
}

static std::istream & parse_string(std::istream &is, std::string &string) {
	string.clear();
	is >> std::ws;
	if (is.peek() != '"') {
		throw std::ios_base::failure("expected string");
	}
	is.get();
	for (int c; (c = is.get()) != '"';) {
		if (!is) {
			throw std::ios_base::failure("unterminated string");
		}
		if (c == '\\') {
			switch (is.get()) {
				case 'b': // backspace (U+0008)
					string.push_back(0x08);
					break;
				case 't': // character tabulation (U+0009)
					string.push_back(0x09);
					break;
				case 'n': // line feed (U+000A)
					string.push_back(0x0A);
					break;
				case 'f': // form feed (U+000C)
					string.push_back(0x0C);
					break;
				case 'r': // carriage return (U+000D)
					string.push_back(0x0D);
					break;
				case '"': // quotation mark (U+0022)
					string.push_back('"');
					break;
				case '/': // solidus (U+002F)
					string.push_back('/');
					break;
				case '\\': // reverse solidus (U+005C)
					string.push_back('\\');
					break;
				case 'u': {
					unsigned int cu = from_hex(is) << 12 | from_hex(is) << 8 | from_hex(is) << 4 | from_hex(is);
					if (cu <= 0x7F) {
						string.push_back(static_cast<char>(cu));
					}
					else {
						if (cu <= 0x07FF) {
							string.push_back(static_cast<char>(0xC0 | cu >> 6));
						}
						else {
							string.push_back(static_cast<char>(0xE0 | cu >> 12));
							string.push_back(static_cast<char>(0x80 | cu >> 6 & (1 << 6) - 1));
						}
						string.push_back(static_cast<char>(0x80 | cu & (1 << 6) - 1));
					}
					break;
				}
				default:
					throw std::ios_base::failure("invalid escape sequence");
			}
		}
		else {
			string.push_back(std::istream::traits_type::to_char_type(c));
		}
	}
	return is;
}

static std::istream & parse_object(std::istream &is, Object::map_t &map) {
	map.clear();
	is >> std::ws;
	if (is.peek() != '{') {
		throw std::ios_base::failure("expected object");
	}
	is.get();
	for (;;) {
		is >> std::ws;
		if (is.peek() == '}') {
			is.get();
			return is;
		}
		if (map.empty()) {
			if (is.peek() != '"') {
				throw std::ios_base::failure("expected string or closing brace");
			}
		}
		else if (is.peek() != ',') {
			throw std::ios_base::failure("expected comma or closing brace");
		}
		else {
			is.get();
		}
		std::string key;
		parse_string(is, key);
		is >> std::ws;
		if (is.peek() != ':') {
			throw std::ios_base::failure("expected colon");
		}
		is.get();
		ValuePtr value;
		is >> value;
		// [C++11] map.emplace(std::move(key), std::move(value));
		map.insert(std::make_pair(std::move(key), std::move(value)));
	}
}

static std::istream & parse_array(std::istream &is, Array::vector_t &vector) {
	vector.clear();
	is >> std::ws;
	if (is.peek() != '[') {
		throw std::ios_base::failure("expected array");
	}
	is.get();
	for (;;) {
		is >> std::ws;
		if (is.peek() == ']') {
			is.get();
			return is;
		}
		if (!vector.empty()) {
			if (is.peek() != ',') {
				throw std::ios_base::failure("expected comma or closing bracket");
			}
			else {
				is.get();
			}
		}
		ValuePtr value;
		is >> value;
		vector.push_back(std::move(value));
	}
}

static std::istream & parse_integer(std::istream &is, intmax_t &value) {
	return is >> value;
}

static std::istream & parse_real(std::istream &is, double &value) {
	return is >> value;
}

static std::istream & collect_digits(std::istream &is, std::string &str, size_t limit = std::numeric_limits<size_t>::max()) {
	if (!::isdigit(is.peek())) {
		throw std::ios_base::failure("expected digit");
	}
	do {
		if (str.size() > limit) {
			throw std::ios_base::failure("too many digits");
		}
		str.push_back(std::istream::traits_type::to_char_type(is.get()));
	} while (::isdigit(is.peek()));
	return is;
}

std::istream & operator >> (std::istream &is, ValuePtr &value) {
	is >> std::ws;
	switch (is.peek()) {
		case '{': {
			Object object;
			parse_object(is, *object);
			value.reset(new Object(std::move(object)));
			return is;
		}
		case '[': {
			Array array;
			parse_array(is, *array);
			value.reset(new Array(std::move(array)));
			return is;
		}
		case '-': case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9': {
			std::string str;
			str.reserve(32);
			if (is.peek() == '-') {
				str.push_back(std::istream::traits_type::to_char_type(is.get()));
			}
			if (is.peek() == '0') {
				str.push_back(std::istream::traits_type::to_char_type(is.get()));
			}
			else {
				collect_digits(is, str, 32);
			}
			bool fractional = false;
			if (is.peek() == '.') {
				str.push_back(std::istream::traits_type::to_char_type(is.get()));
				fractional = true;
				collect_digits(is, str, 32);
			}
			if ((is.peek() | 0x20) == 'e') {
				str.push_back(std::istream::traits_type::to_char_type(is.get()));
				fractional = true;
				if (is.peek() == '-' || is.peek() == '+') {
					str.push_back(std::istream::traits_type::to_char_type(is.get()));
				}
				collect_digits(is, str, 32);
			}
			std::stringstream ss(str);
			if (fractional) {
				Real real;
				parse_real(ss, *real);
				value.reset(new Real(std::move(real)));
			}
			else {
				Integer integer;
				parse_integer(ss, *integer);
				value.reset(new Integer(std::move(integer)));
			}
			return is;
		}
		case '"': {
			String string;
			parse_string(is, *string);
			value.reset(new String(std::move(string)));
			return is;
		}
		case 't':
			is.get();
			if (is.get() == 'r' && is.get() == 'u' && is.get() == 'e') {
				value.reset(new Boolean(true));
				return is;
			}
			break;
		case 'f':
			is.get();
			if (is.get() == 'a' && is.get() == 'l' && is.get() == 's' && is.get() == 'e') {
				value.reset(new Boolean(false));
				return is;
			}
			break;
		case 'n':
			is.get();
			if (is.get() == 'u' && is.get() == 'l' && is.get() == 'l') {
				value.reset();
				return is;
			}
			break;
	}
	throw std::ios_base::failure("expected object, array, number, string, boolean, or null");
}

std::ostream & operator << (std::ostream &os, const Value &value) {
	return value.format(os);
}

std::ostream & operator << (std::ostream &os, const ValuePtr &value) {
	return value ? value->format(os) : os << "null";
}


const Value * find(const Object &object, const std::string &key) {
	auto it = object->find(key);
	return it == object->end() ? nullptr : it->second.get();
}

const Value & get(const Object &object, const std::string &key) {
	auto ptr = find(object, key);
	if (!ptr) {
		throw std::invalid_argument(key + " missing");
	}
	return *ptr;
}

const Object & as_object(const Value &value) {
	auto object_ptr = dynamic_cast<const Object *>(&value);
	if (!object_ptr) {
		throw std::invalid_argument("expected object");
	}
	return *object_ptr;
}

const Array & as_array(const Value &value) {
	auto array_ptr = dynamic_cast<const Array *>(&value);
	if (!array_ptr) {
		throw std::invalid_argument("expected array");
	}
	return *array_ptr;
}

const Number & as_number(const Value &value) {
	auto number_ptr = dynamic_cast<const Number *>(&value);
	if (!number_ptr) {
		throw std::invalid_argument("expected number");
	}
	return *number_ptr;
}

const Integer & as_integer(const Value &value) {
	auto integer_ptr = dynamic_cast<const Integer *>(&value);
	if (!integer_ptr) {
		throw std::invalid_argument("expected integer");
	}
	return *integer_ptr;
}

const String & as_string(const Value &value) {
	auto string_ptr = dynamic_cast<const String *>(&value);
	if (!string_ptr) {
		throw std::invalid_argument("expected string");
	}
	return *string_ptr;
}

const Boolean & as_boolean(const Value &value) {
	auto boolean_ptr = dynamic_cast<const Boolean *>(&value);
	if (!boolean_ptr) {
		throw std::invalid_argument("expected boolean");
	}
	return *boolean_ptr;
}

} // namespace json
