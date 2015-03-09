#pragma once
#include "DeclVal.h"
#include "Type.h"
template <class F, class ...A> constexpr auto ResultType(Type<F> f, Type<A>... a) {
	return Type<decltype(DeclVal(f)(DeclVal(a)...))>{};
}
