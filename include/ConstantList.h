#pragma once
#include "Unit.h"
template <class... T> struct ConstantList : Unit<ConstantList<T...>> {
	template <class N> constexpr auto operator[](N) const { return GetNthArg(N{}, T{}...); }
};
