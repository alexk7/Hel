#pragma once
#include "Unit.h"
template <class T, T t> struct Constant : Unit<Constant<T, t>> {
	constexpr operator T() const { return t; }
};
