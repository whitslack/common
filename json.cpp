#include "json.h"

#include <cctype>
#include <cmath>
#include <iomanip>
#include <limits>
#include <sstream>
#include <stdexcept>

namespace json {

static std::ostream & format_string(std::ostream &os, const std::string &str);


Object & _noreturn Value::as_object() {
	throw std::invalid_argument("expected object");
}

const Object & _noreturn Value::as_object() const {
	throw std::invalid_argument("expected object");
}

Array & _noreturn Value::as_array() {
	throw std::invalid_argument("expected array");
}

const Array & _noreturn Value::as_array() const {
	throw std::invalid_argument("expected array");
}

Number & _noreturn Value::as_number() {
	throw std::invalid_argument("expected number");
}

const Number & _noreturn Value::as_number() const {
	throw std::invalid_argument("expected number");
}

Integer & _noreturn Value::as_integer() {
	throw std::invalid_argument("expected integer");
}

const Integer & _noreturn Value::as_integer() const {
	throw std::invalid_argument("expected integer");
}

String & _noreturn Value::as_string() {
	throw std::invalid_argument("expected string");
}

const String & _noreturn Value::as_string() const {
	throw std::invalid_argument("expected string");
}

Boolean & _noreturn Value::as_boolean() {
	throw std::invalid_argument("expected boolean");
}

const Boolean & _noreturn Value::as_boolean() const {
	throw std::invalid_argument("expected boolean");
}


const Value * Object::find(const std::string &key) const {
	auto itr = map.find(key);
	return itr == map.end() ? nullptr : itr->second.get();
}

const Value & Object::get(const std::string &key) const {
	auto ptr = this->find(key);
	if (!ptr) {
		throw std::invalid_argument(key + " missing");
	}
	return *ptr;
}

Object & Object::as_object() {
	return *this;
}

const Object & Object::as_object() const {
	return *this;
}

std::ostream & Object::format(std::ostream &os) const {
	os.put('{');
	auto it = map.begin();
	if (it != map.end()) {
		for (;;) {
			format_string(os, it->first).put(':') << it->second;
			if (++it == map.end()) {
				break;
			}
			os.put(',');
		}
	}
	return os.put('}');
}


Array & Array::as_array() {
	return *this;
}

const Array & Array::as_array() const {
	return *this;
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


Number & Number::as_number() {
	return *this;
}

const Number & Number::as_number() const {
	return *this;
}


Integer & Integer::as_integer() {
	return *this;
}

const Integer & Integer::as_integer() const {
	return *this;
}

std::ostream & Integer::format(std::ostream &os) const {
	return os << value;
}


std::ostream & Real::format(std::ostream &os) const {
	return std::isfinite(value) ? os << value : os << "null";
}


String & String::as_string() {
	return *this;
}

const String & String::as_string() const {
	return *this;
}

std::ostream & String::format(std::ostream &os) const {
	return format_string(os, string);
}

static std::ostream & format_string(std::ostream &os, const std::string &str) {
	static const char HEX[] = "0123456789ABCDEF";
	os.put('"');
	for (char c : str) {
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


Boolean & Boolean::as_boolean() {
	return *this;
}

const Boolean & Boolean::as_boolean() const {
	return *this;
}

std::ostream & Boolean::format(std::ostream &os) const {
	return os << (value ? "true" : "false");
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
		map.emplace(std::move(key), std::move(value));
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
		vector.emplace_back(std::move(value));
	}
}

static std::istream & parse_integer(std::istream &is, intmax_t &value) {
	return is >> value;
}

static std::istream & parse_real(std::istream &is, double &value) {
	return is >> value;
}

static std::istream & copy_digits(std::ostream &os, std::istream &is, size_t limit = std::numeric_limits<size_t>::max()) {
	if (!::isdigit(is.peek())) {
		throw std::ios_base::failure("expected digit");
	}
	do {
		if (limit-- == 0) {
			throw std::ios_base::failure("too many digits");
		}
		os.put(std::istream::traits_type::to_char_type(is.get()));
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
			std::stringstream ss;
			if (is.peek() == '-') {
				ss.put(std::istream::traits_type::to_char_type(is.get()));
			}
			if (is.peek() == '0') {
				ss.put(std::istream::traits_type::to_char_type(is.get()));
			}
			else {
				copy_digits(ss, is, 32);
			}
			bool fractional = false;
			if (is.peek() == '.') {
				ss.put(std::istream::traits_type::to_char_type(is.get()));
				fractional = true;
				copy_digits(ss, is, 32);
			}
			if ((is.peek() | 0x20) == 'e') {
				ss.put(std::istream::traits_type::to_char_type(is.get()));
				fractional = true;
				if (is.peek() == '-' || is.peek() == '+') {
					ss.put(std::istream::traits_type::to_char_type(is.get()));
				}
				copy_digits(ss, is, 32);
			}
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
			is.setstate(ss.rdstate() & ~std::ios_base::eofbit);
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
	auto orig_flags = os.flags(std::ios_base::dec);
	auto orig_precision = os.precision(std::numeric_limits<double>::digits10);
	os.width(0);
	value.format(os);
	os.flags(orig_flags);
	os.precision(orig_precision);
	return os;
}

std::ostream & operator << (std::ostream &os, const ValuePtr &value) {
	return value ? os << *value : os << "null";
}

} // namespace json
