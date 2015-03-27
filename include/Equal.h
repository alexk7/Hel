#pragma once
#include "ConstantFunction.h"
constexpr static struct Equal_t : ConstantFunction<Equal_t> {
	template <class T, class U> constexpr static auto invoke(const T& t, const U& u) { return t == u; }
} Equal{};
