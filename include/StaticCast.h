#pragma once
#include "Type.h"
template <class To, class From> auto StaticCast(Type<To>, From from) { return static_cast<To>(from); }
