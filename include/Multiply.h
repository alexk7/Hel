#pragma once
#include "ConstantFunction.h"
#include "Noop.h"
struct Multiply_t {
	template <class T, class ...U> constexpr static auto invoke(T t, const U& ...u) {
		(void)(T[]){(t *= u)...};
		return t;
	}
};
constexpr static ConstantFunction<Multiply_t> Multiply{};
