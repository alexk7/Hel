#pragma once
#include "Begin.h"
#include "End.h"
template <class T> constexpr auto Empty(const T& r) { return Begin(r) == End(r); }
