#pragma once
#include "Constant.h"
template <class D> struct Unit : Constant<D> {
	constexpr static auto Value() { return D{}; }
};
