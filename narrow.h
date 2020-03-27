#pragma once

#include <limits>
#include <stdexcept>

#include "compiler.h"


template <typename I, typename J>
static std::enable_if_t<std::is_signed_v<I> && std::is_signed_v<J>, I> _const narrow_check(J value) {
	if (_unlikely(value < std::numeric_limits<I>::min() || value > std::numeric_limits<I>::max())) {
		throw std::out_of_range("out of range");
	}
	return static_cast<I>(value);
}

template <typename I, typename J>
static std::enable_if_t<std::is_signed_v<I> && std::is_unsigned_v<J>, I> _const narrow_check(J value) {
	if (_unlikely(value > static_cast<std::make_unsigned_t<I>>(std::numeric_limits<I>::max()))) {
		throw std::out_of_range("out of range");
	}
	return static_cast<I>(value);
}

template <typename I, typename J>
static std::enable_if_t<std::is_unsigned_v<I> && std::is_signed_v<J>, I> _const narrow_check(J value) {
	if (_unlikely(value < 0 || static_cast<std::make_unsigned_t<J>>(value) > std::numeric_limits<I>::max())) {
		throw std::out_of_range("out of range");
	}
	return static_cast<I>(value);
}

template <typename I, typename J>
static std::enable_if_t<std::is_unsigned_v<I> && std::is_unsigned_v<J>, I> _const narrow_check(J value) {
	if (_unlikely(value > std::numeric_limits<I>::max())) {
		throw std::out_of_range("out of range");
	}
	return static_cast<I>(value);
}


template <typename I, typename J>
static std::enable_if_t<std::is_signed_v<I> && std::is_signed_v<J>, I> _const saturate(J value) noexcept {
	if (_unlikely(value < std::numeric_limits<I>::min())) {
		return std::numeric_limits<I>::min();
	}
	if (_unlikely(value > std::numeric_limits<I>::max())) {
		return std::numeric_limits<I>::max();
	}
	return static_cast<I>(value);
}

template <typename I, typename J>
static std::enable_if_t<std::is_signed_v<I> && std::is_unsigned_v<J>, I> _const saturate(J value) noexcept {
	if (_unlikely(value > static_cast<std::make_unsigned_t<I>>(std::numeric_limits<I>::max()))) {
		return std::numeric_limits<I>::max();
	}
	return static_cast<I>(value);
}

template <typename I, typename J>
static std::enable_if_t<std::is_unsigned_v<I> && std::is_signed_v<J>, I> _const saturate(J value) noexcept {
	if (_unlikely(value < 0)) {
		return 0;
	}
	if (_unlikely(static_cast<std::make_unsigned_t<J>>(value) > std::numeric_limits<I>::max())) {
		return std::numeric_limits<I>::max();
	}
	return static_cast<I>(value);
}

template <typename I, typename J>
static std::enable_if_t<std::is_unsigned_v<I> && std::is_unsigned_v<J>, I> _const saturate(J value) noexcept {
	if (_unlikely(value > std::numeric_limits<I>::max())) {
		return std::numeric_limits<I>::max();
	}
	return static_cast<I>(value);
}
