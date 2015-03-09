#pragma once
#include "Unit.h"
template <class T> struct Type : Unit<Type<T>> {};
