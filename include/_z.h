#pragma once
#include "CharConstant.h"
#include "ConstantList.h"
#include "TConstant.h"
#include "Type.h"
template <class T, char c> constexpr auto Parse(Type<T>, ConstantList<CharConstant<c>>) {
	return TConstant<T, c - '0'>{};
}
template <char... c> constexpr auto operator""_z() { return Parse(Type<size_t>{}, ConstantList<CharConstant<c>...>{}); }
