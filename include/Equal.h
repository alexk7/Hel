#pragma once
#include "ConstantFunction.h"
struct Equal_t {
	template <class T, class U> constexpr static auto invoke(const T& t, const U& u) { return t == u; }
};
constexpr static ConstantFunction<Equal_t> Equal{};
