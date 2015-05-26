#pragma once
#include "Constant.h"
#include "ConstantList.h"
template <class... T> constexpr auto MakeList(Constant<T>...) { return ConstantList<T...>{}; }
