#pragma once
#include "Unit.h"
template <class ...T> struct UnitList : Unit<UnitList<T...>> {
	template <class N> auto operator[](N n) { return GetNthArg(n, T{}...); }
};
