#pragma once
#include "Type.h"
template <class T> constexpr auto RemoveRValueReference(Type<T&&>) { return Type<T>{}; }
template <class T> constexpr auto RemoveRValueReference(Type<T>) { return Type<T>{}; }
