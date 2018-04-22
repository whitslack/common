#include <cstring>
#include <string_view>

#include "compiler.h"

template <typename Itr1, typename Itr2>
static int _pure compare_ci(Itr1 itr1, Itr1 end1, Itr2 itr2, Itr2 end2) {
	for (;;) {
		if (itr1 == end1) {
			return itr2 == end2 ? 0 : -1;
		}
		if (itr2 == end2) {
			return 1;
		}
		auto c1 = std::toupper(*itr1++), c2 = std::toupper(*itr2++);
		if (c1 != c2) {
			return c1 < c2 ? -1 : 1;
		}
	}
}

template <typename Itr1>
static inline int _pure compare_ci(Itr1 itr1, Itr1 end1, std::string_view str2) {
	return compare_ci(itr1, end1, str2.begin(), str2.end());
}

template <typename Itr2>
static inline int _pure compare_ci(std::string_view str1, Itr2 itr2, Itr2 end2) {
	return compare_ci(str1.begin(), str1.end(), itr2, end2);
}

static inline int _pure compare_ci(std::string_view str1, std::string_view str2) noexcept {
	return compare_ci(str1.begin(), str1.end(), str2.begin(), str2.end());
}

struct less_ci {
	bool _pure operator () (std::string_view str1, std::string_view str2) const noexcept {
		return compare_ci(str1, str2) < 0;
	}
};

struct greater_ci {
	bool _pure operator () (std::string_view str1, std::string_view str2) const noexcept {
		return compare_ci(str1, str2) > 0;
	}
};
