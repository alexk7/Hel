#pragma once
#include "Type.h"
#include <type_traits>
template <class T, class U> std::enable_if_t<type<T> == type<U&&>, T> UnsafeGetAs(Type<T>, U&& u) {
	return static_cast<T>(u);
}
