#pragma once
#include "GetNthArg.h"
template <class N, class... X> auto GetNthElement(N n, ConstantList<X...> xl) {
	return GetNthArg(n, X{}...);
};
