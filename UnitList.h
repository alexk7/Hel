#pragma once
#include "Unit.h"
template <class ...T> struct UnitList : Unit<UnitList<T...>> {};
