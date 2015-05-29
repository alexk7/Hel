#pragma once
#include "ConstantList.h"
#include "SizeConstant.h"
template <class... T> constexpr auto Size(const ConstantList<T...>&) { return SizeConstant<sizeof...(T)>{}; }
