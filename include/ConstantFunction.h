#pragma once
#include "Constant.h"
template <class F> struct ConstantFunction {
	template <class ...T> struct Result {
		constexpr static auto value = F::invoke(T::value...);
	};
	template <class T, class ...U> constexpr auto operator()(Constant<T>, Constant<U>...) const {
		return GetConstant(Result<T, U...>{});
	}
	template <class T, class ...U> constexpr auto operator()(T t, U ...u) const {
		return F::invoke(t, u...);
	}
};
