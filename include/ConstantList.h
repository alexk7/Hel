#pragma once
#include "Unit.h"
template <class... T> struct ConstantList : Unit<ConstantList<T...>> {
	template <class N> constexpr auto operator[](N) const { return GetNthElement(N{}, ConstantList{}); }
};
template <class F, class... T> constexpr auto operator|(ConstantList<T...>, F&& f) {
	return static_cast<F&&>(f)(T{}...);
}
#include "ConstantList.hpp"
