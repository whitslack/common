#include <cctype>
#include <cwctype>
#include <string_view>

#include "compiler.h"


namespace ci {

namespace {
	template <typename> struct __toupper;
	template <> struct __toupper<char> : std::integral_constant<int (*)(int) noexcept, &std::toupper> { };
	template <> struct __toupper<wchar_t> : std::integral_constant<wint_t (*)(wint_t) noexcept, &std::towupper> { };
#if WINT_WIDTH >= UINT16_WIDTH
	template <> struct __toupper<char16_t> : std::integral_constant<wint_t (*)(wint_t) noexcept, &std::towupper> { };
#endif
#if WINT_WIDTH >= UINT32_WIDTH
	template <> struct __toupper<char32_t> : std::integral_constant<wint_t (*)(wint_t) noexcept, &std::towupper> { };
#endif
}

template <typename CharT>
struct char_traits : std::char_traits<CharT> {
	using typename std::char_traits<CharT>::char_type;
	using typename std::char_traits<CharT>::int_type;
	using std::char_traits<CharT>::to_int_type;
	static bool _const eq(char_type a, char_type b) noexcept { return __toupper<CharT>()(to_int_type(a)) == __toupper<CharT>()(to_int_type(b)); }
	static bool _const lt(char_type a, char_type b) noexcept { return __toupper<CharT>()(to_int_type(a)) < __toupper<CharT>()(to_int_type(b)); }
	static int _const compare(char_type a, char_type b) noexcept { return __toupper<CharT>()(to_int_type(a)) - __toupper<CharT>()(to_int_type(b)); }
	static int _pure compare(const char_type s1[], const char_type s2[], std::size_t count) noexcept {
		for (std::size_t i = 0; i < count; ++i) if (s1[i] != s2[i]) if (int c = compare(s1[i], s2[i])) return c;
		return 0;
	}
	static const char_type * _pure find(const char_type s[], std::size_t count, const char_type &ch) noexcept {
		auto u = __toupper<CharT>()(to_int_type(ch));
		for (const char_type *e = s + count; s != e; ++s) if (__toupper<CharT>()(to_int_type(*s)) == u) return s;
		return nullptr;
	}
	static bool _const eq_int_type(int_type c1, int_type c2) noexcept { return __toupper<CharT>()(c1) == __toupper<CharT>()(c2); }
};

using string = std::basic_string<char, char_traits<char>>;
using wstring = std::basic_string<wchar_t, char_traits<wchar_t>>;
using u16string = std::basic_string<char16_t, char_traits<char16_t>>;
using u32string = std::basic_string<char32_t, char_traits<char32_t>>;

using string_view = std::basic_string_view<char, char_traits<char>>;
using wstring_view = std::basic_string_view<wchar_t, char_traits<wchar_t>>;
using u16string_view = std::basic_string_view<char16_t, char_traits<char16_t>>;
using u32string_view = std::basic_string_view<char32_t, char_traits<char32_t>>;

inline namespace literals {
	static constexpr string_view _const operator ""_ci (const char data[], size_t size) noexcept { return { data, size}; }
	static constexpr wstring_view _const operator ""_ci (const wchar_t data[], size_t size) noexcept { return { data, size}; }
	static constexpr u16string_view _const operator ""_ci (const char16_t data[], size_t size) noexcept { return { data, size}; }
	static constexpr u32string_view _const operator ""_ci (const char32_t data[], size_t size) noexcept { return { data, size}; }
}

#define _(CharT, NS1, NS2) \
	static inline int _pure compare(std::basic_string_view<CharT, NS1::char_traits<CharT>> str1, std::basic_string_view<CharT, NS2::char_traits<CharT>> str2) noexcept { \
		int c = ci::char_traits<CharT>::compare(str1.data(), str2.data(), std::min(str1.size(), str2.size())); \
		return c ? c : (str1.size() > str2.size()) - (str1.size() < str2.size()); \
	}
#define __(CharT) \
	_(CharT, std, std) \
	_(CharT, std, ci) \
	_(CharT, ci, std) \
	_(CharT, ci, ci)
__(char)
__(wchar_t)
__(char16_t)
__(char32_t)
#undef __
#undef _

#define _(name, Op) \
	struct name { \
		typedef void is_transparent; \
		template <typename... Args> \
		bool _pure operator () (Args &&...args) const noexcept { \
			return ci::compare(std::forward<Args>(args)...) Op 0; \
		} \
	};
_(equal_to, ==)
_(not_equal_to, !=)
_(less, <)
_(greater, >)
_(less_equal, <=)
_(greater_equal, >=)
#undef _

} // namespace ci


#define _(CharT, Op) \
	static inline bool _pure operator Op (std::basic_string_view<CharT, std::char_traits<CharT>> str1, std::basic_string_view<CharT, ci::char_traits<CharT>> str2) noexcept { \
		return ci::compare(str1, str2) Op 0; \
	} \
	\
	static inline bool _pure operator Op (std::basic_string_view<CharT, ci::char_traits<CharT>> str1, std::basic_string_view<CharT, std::char_traits<CharT>> str2) noexcept { \
		return ci::compare(str1, str2) Op 0; \
	}
#define __(CharT) \
	_(CharT, ==) \
	_(CharT, !=) \
	_(CharT, <) \
	_(CharT, >) \
	_(CharT, <=) \
	_(CharT, >=)
__(char)
__(wchar_t)
__(char16_t)
__(char32_t)
#undef __
#undef _
