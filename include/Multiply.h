#pragma once
#include "ConstantFunction.h"
#include "Noop.h"
constexpr struct Multiply_t : ConstantFunction<Multiply_t> {
	using ConstantFunction::operator();
	template <class T, class ...U> constexpr T operator()(T t, const U& ...u) const {
		Noop((t *= u)...);
		return t;
	}
} Multiply{};
