#pragma once
#include "Constant.h"
#include "TypedConstant.h"
template <class F> struct ConstantFunction {
	template <class ...T> struct Result {
		constexpr static auto value = F::invoke(T::value...);
	};
	template <class R> constexpr static auto GetConstant(R) { return TypedConstant<decltype(R::value), R::value>{}; }
	template <class R> constexpr static auto GetConstant(...) { return Constant<R>{}; }
	template <class T, class ...U> constexpr auto operator()(Constant<T>, Constant<U>...) const {
		using R = Result<T, U...>;
		return GetConstant<R>(R{});
	}
	template <class T, class ...U> constexpr auto operator()(T t, U ...u) const {
		return F::invoke(t, u...);
	}
};
