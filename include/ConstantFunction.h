#pragma once
#include "Constant.h"
template <class D>
struct ConstantFunction {
	template <class ...T>
	struct Result {
		constexpr static auto value = D{}(T::value...);
	};
	template <class T, class ...U> constexpr auto operator()(Constant<T>, Constant<U>...) const {
		return Constant<Result<T, U...>>{};
	}
};
