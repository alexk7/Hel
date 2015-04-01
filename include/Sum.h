#pragma once
#include "ConstantFunction.h"
#include "Noop.h"
constexpr struct Sum_t : ConstantFunction<Sum_t> {
	template <class T, class ...U> constexpr static auto invoke(T t, const U& ...u) {
		(void)(T[]){(t += u)...};
		return t;
	}
} Sum{};
