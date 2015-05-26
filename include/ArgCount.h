#pragma once
#include "SizeConstant.h"
template <class... T> constexpr auto ArgCount(const T&...) { return SizeConstant<sizeof...(T)>{}; }
