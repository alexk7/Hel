#pragma once
#include "Type.h"
#include <type_traits>
template <class T> constexpr auto Decay(Type<T>) { return Type<std::decay_t<T>>{}; }
