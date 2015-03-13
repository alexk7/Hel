#pragma once
#include "Constant.h"
template <class Derived>
struct ConstantFunction {
	template <class T, T t1, T ...t2> constexpr auto operator()(Constant<T, t1>, Constant<T, t2>...) const {
		constexpr auto result = Derived{}(t1, t2...);
		return Constant<decltype(result), result>{};
	}
};
