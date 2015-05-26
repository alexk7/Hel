#pragma once
#include "ArgCount.h"
#include "CONSTANT_ASSERT.h"
#include "MakeConstantArray.h"
#include "StaticCast.h"
#include "Type.h"
template <class N, class... X> auto GetNthArg(N n, X... x) {
	CONSTANT_ASSERT(n < ArgCount(x...));
	auto types = MakeConstantArray(Type<X*>{}...);
	void* values[] = { &x... };
	return *StaticCast(types[n], values[n]);
};
