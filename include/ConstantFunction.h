#pragma once
#include "Constant.h"
template <size_t p> struct SfinaePriority : SfinaePriority<p - 1> {};
template <> struct SfinaePriority<0> {};
template <class F> class ConstantFunction {
	template <class... T> struct Result {
		constexpr static auto value = F::invoke(T::value...);
	};
	//*
	template <class... T> constexpr static auto SfinaeHack(SfinaePriority<2>, T...)
		-> TypedConstant<std::remove_const_t<decltype(F::invoke(T::value...))>, F::invoke(T::value...)> {
		return {};
	}
	template <class... T> constexpr static auto SfinaeHack(SfinaePriority<1>, T...)
		-> TypedConstant<const decltype(F::invoke(T::value...))&, Result<T...>::value> {
		return {};
	}
	template <class... T> constexpr static auto SfinaeHack(SfinaePriority<0>, T... t) {
		return F::invoke(t...);
	}
	//*/
public:
	template <class T, class ...U> constexpr auto operator()(T t, U ...u) const {
		return SfinaeHack(SfinaePriority<2>{}, t, u...);
	}
};
