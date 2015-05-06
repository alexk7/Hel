#pragma once
#include "TypeName.h"
#include "Unit.h"
template <class T> struct Type : Unit<Type<T>> {};
template <class T> constexpr bool operator==(Type<T>, Type<T>) { return true; }
template <class A, class B> constexpr auto operator==(Type<A>, Type<B>) { return false; }
template <class T> constexpr static Type<T> type{};
template <class T> std::string Name(Type<T>) { return TypeName(typeid(T)); }
