#pragma once
#include "ConstantFunction.h"
struct Equal_t {
	template <class T, class... U> constexpr static bool invoke(const T& t, const U&... u) {
		bool b = true;
		(void)(bool[]){(b = b && (t == u))...};
		return b;
	}
};
constexpr static ConstantFunction<Equal_t> Equal{};
