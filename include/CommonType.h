#pragma once
#include "DeclVal.h"
#include "Type.h"
template <class T> constexpr auto CommonType(Type<T> t) { return t; }
template <class T, class... U> constexpr auto CommonType(Type<T>, Type<void>, Type<U>...) { return type<void>; }
template <class T, class... U> constexpr auto CommonType(Type<void>, Type<T>, Type<U>...) { return type<void>; }
template <class T, class U, class... V> constexpr auto CommonType(Type<T> t, Type<U> u, Type<V>... v) {
	return CommonType(Type<decltype(false ? DeclVal(t) : DeclVal(u))>{}, v...);
}
