#pragma once
#include "ConstantFunction.h"
struct FindArg_t {
	template <class T, class... X> constexpr static auto invoke(T t, X... x) {
		size_t i = 0;
		bool done = false;
 		(void)(size_t[]){ 0, (done ? 0 : t == x ? done = true : ++i)... };
		return i;
	}
};
constexpr static ConstantFunction<FindArg_t> FindArg{};
