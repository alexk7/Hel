#pragma once
#include "Unit.h"
#include "True.h"
#include "False.h"
template <class T> struct Type : Unit<Type<T>> {};
template <class T> constexpr auto operator==(Type<T>, Type<T>) { return True; }
template <class A, class B> constexpr auto operator==(Type<A>, Type<B>) { return False; }
