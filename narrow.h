#pragma once

#include <limits>
#include <stdexcept>

template <typename I, typename J>
static typename std::enable_if<std::is_signed<I>::value && std::is_signed<J>::value, I>::type narrow_check(J value) {
	if (value < std::numeric_limits<I>::min() || value > std::numeric_limits<I>::max()) {
		throw std::out_of_range("out of range");
	}
	return static_cast<I>(value);
}

template <typename I, typename J>
static typename std::enable_if<std::is_signed<I>::value && std::is_unsigned<J>::value, I>::type narrow_check(J value) {
	if (value > static_cast<typename std::make_unsigned<I>::type>(std::numeric_limits<I>::max())) {
		throw std::out_of_range("out of range");
	}
	return static_cast<I>(value);
}

template <typename I, typename J>
static typename std::enable_if<std::is_unsigned<I>::value && std::is_signed<J>::value, I>::type narrow_check(J value) {
	if (value < 0 || static_cast<typename std::make_unsigned<J>::type>(value) > std::numeric_limits<I>::max()) {
		throw std::out_of_range("out of range");
	}
	return static_cast<I>(value);
}

template <typename I, typename J>
static typename std::enable_if<std::is_unsigned<I>::value && std::is_unsigned<J>::value, I>::type narrow_check(J value) {
	if (value > std::numeric_limits<I>::max()) {
		throw std::out_of_range("out of range");
	}
	return static_cast<I>(value);
}


template <typename I, typename J>
static typename std::enable_if<std::is_signed<I>::value && std::is_signed<J>::value, I>::type saturate(J value) {
	if (value < std::numeric_limits<I>::min()) {
		return std::numeric_limits<I>::min();
	}
	if (value > std::numeric_limits<I>::max()) {
		return std::numeric_limits<I>::max();
	}
	return static_cast<I>(value);
}

template <typename I, typename J>
static typename std::enable_if<std::is_signed<I>::value && std::is_unsigned<J>::value, I>::type saturate(J value) {
	if (value > static_cast<typename std::make_unsigned<I>::type>(std::numeric_limits<I>::max())) {
		return std::numeric_limits<I>::max();
	}
	return static_cast<I>(value);
}

template <typename I, typename J>
static typename std::enable_if<std::is_unsigned<I>::value && std::is_signed<J>::value, I>::type saturate(J value) {
	if (value < 0) {
		return 0;
	}
	if (static_cast<typename std::make_unsigned<J>::type>(value) > std::numeric_limits<I>::max()) {
		return std::numeric_limits<I>::max();
	}
	return static_cast<I>(value);
}

template <typename I, typename J>
static typename std::enable_if<std::is_unsigned<I>::value && std::is_unsigned<J>::value, I>::type saturate(J value) {
	if (value > std::numeric_limits<I>::max()) {
		return std::numeric_limits<I>::max();
	}
	return static_cast<I>(value);
}
