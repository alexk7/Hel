#pragma once
#include "ConstantFunction.h"
#include "True.h"
constexpr struct And_t : ConstantFunction<And_t> {
	constexpr static auto invoke() { return True; }
	template <class T, class ...U> constexpr static auto invoke(const T& t, const U&... u) {
		bool b = static_cast<bool>(t);
		(void)(bool[]){(b = b && u)...};
		return b;
	}
} And{};
