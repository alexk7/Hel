#pragma once
#include "Recursive.h"
template <class F> constexpr auto MakeRecursive(F&& f) { return Recursive<std::decay_t<F>>{static_cast<F&&>(f)}; }
