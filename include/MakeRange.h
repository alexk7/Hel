#pragma once
#include "Range.h"
constexpr static struct MakeRange_t {
	template <class T, class U> constexpr auto operator()(T&& t, U&& u) const {
		return Range<T, U>{ static_cast<T&&>(t), static_cast<U&&>(u) };
	}
} MakeRange{};
