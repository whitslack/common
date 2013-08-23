#include <cstring>

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
static int _pure compare_ci(Itr1 itr1, Itr1 end1, const char str2[]) {
	return compare_ci(itr1, end1, str2, str2 + std::strlen(str2));
}

template <typename Itr1>
static int _pure compare_ci(Itr1 itr1, Itr1 end1, const std::string &str2) {
	return compare_ci(itr1, end1, str2.begin(), str2.end());
}

template <typename Itr2>
static int _pure compare_ci(const char str1[], Itr2 itr2, Itr2 end2) {
	return compare_ci(str1, str1 + std::strlen(str1), itr2, end2);
}

template <typename Itr2>
static int _pure compare_ci(const std::string &str1, Itr2 itr2, Itr2 end2) {
	return compare_ci(str1.begin(), str1.end(), itr2, end2);
}

template <typename Str1>
static int _pure compare_ci(Str1 &&str1, const char str2[]) {
	return compare_ci(std::forward<Str1>(str1), str2, str2 + std::strlen(str2));
}

template <typename Str1>
static int _pure compare_ci(Str1 &&str1, const std::string &str2) {
	return compare_ci(std::forward<Str1>(str1), str2.begin(), str2.end());
}

struct less_ci {
	template <typename Str1, typename Str2>
	bool _pure operator () (Str1 &&str1, Str2 &&str2) const {
		return compare_ci(std::forward<Str1>(str1), std::forward<Str2>(str2)) < 0;
	}
};

struct greater_ci {
	template <typename Str1, typename Str2>
	bool _pure operator () (Str1 &&str1, Str2 &&str2) const {
		return compare_ci(std::forward<Str1>(str1), std::forward<Str2>(str2)) > 0;
	}
};
