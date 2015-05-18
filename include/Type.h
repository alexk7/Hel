#pragma once
#include "False.h"
#include "TypeName.h"
#include "True.h"
#include "Unit.h"
template <class T> struct Type : Unit<Type<T>> {};
template <class T> constexpr auto operator==(Type<T>, Type<T>) { return True; }
template <class A, class B> constexpr auto operator==(Type<A>, Type<B>) { return False; }
template <class T> constexpr static Type<T> type{};
template <class T> std::string Name(Type<T>) { return TypeName(typeid(T)); }
