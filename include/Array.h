#pragma once
#include "MakeIndexList.h"
#include "SizeConstant.h"
#include <stddef.h>
template <class T, size_t s> struct Array
{
    T values[s];
	constexpr T& operator[](size_t n) { return values[n]; }
	constexpr const T& operator[](size_t n) const { return values[n]; }
	friend constexpr auto Size(const Array&) { return SizeConstant<s>{}; }
};
