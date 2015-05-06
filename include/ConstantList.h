#pragma once
#include "Unit.h"
template <class... T> struct ConstantList : Unit<ConstantList<T...>> {
	template <class N> auto operator[](N n) { return GetNthArg(n, T{}...); }
};
