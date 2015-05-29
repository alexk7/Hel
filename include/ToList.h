#pragma once
#include "Constant.h"
template <class T, class F> constexpr auto ToList(Constant<T>) {
	constexpr auto t = T::Value();
	return MakeIndexList(Size(t)) | [](auto... i) {
		return MakeList(VCONSTANT(t[i]){}...);
	};
}
