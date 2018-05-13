#include "../ci.h"

#include <cassert>
#include <map>
#include <string>

using namespace std::string_literals;
using namespace std::string_view_literals;
using namespace ci::literals;

int main() {
	auto ptr = "Hello world!";
	auto sv = "Hello world!"sv;
	auto str = "Hello world!"s;
	auto sv_ci = "HELLO WORLD!"_ci;
	auto str_ci = ci::string("HELLO WORLD!");

	assert(ptr == ptr);
	assert(ptr == sv);
	assert(ptr == str);
	assert(ptr == sv_ci);
	assert(ptr == str_ci);

	assert(sv == ptr);
	assert(sv == sv);
	assert(sv == str);
	assert(sv == sv_ci);
	assert(sv == str_ci);

	assert(str == ptr);
	assert(str == sv);
	assert(str == str);
	assert(str == sv_ci);
	assert(str == str_ci);

	assert(sv_ci == ptr);
	assert(sv_ci == sv);
	assert(sv_ci == str);
	assert(sv_ci == sv_ci);
	assert(sv_ci == str_ci);

	assert(str_ci == ptr);
	assert(str_ci == sv);
	assert(str_ci == str);
	assert(str_ci == sv_ci);
	assert(str_ci == str_ci);

	std::map<std::string, std::string, ci::less> map { { "foo", "bar" } };
	assert(map.find("Foo") != map.end());

	return 0;
}
