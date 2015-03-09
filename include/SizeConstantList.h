#pragma once
#include "ConstantList.h"
#include "SizeConstant.h"
template <size_t... s> using SizeConstantList = ConstantList<SizeConstant<s>...>;
