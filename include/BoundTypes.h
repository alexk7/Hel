#pragma once
#include "MakeList.h"
#include "Type.h"
template <class T> constexpr auto BoundTypes(Type<T>) { return MakeList(type<T>); }
