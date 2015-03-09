#pragma once
#include "Unit.h"
template <class... T> struct ConstantList : Unit<ConstantList<T...>> {
	template <class N> constexpr auto operator[](N) const { return GetNthElement(N{}, ConstantList{}); }
	template <class F> constexpr auto operator|(F&& f) const {
		return static_cast<F&&>(f)(T{}...);
	}
};
#include "ConstantList.hpp"
