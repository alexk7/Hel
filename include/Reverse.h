#pragma once
#include "Begin.h"
#include "End.h"
#include "MakeRange.h"
#include <iterator>
template <class T> constexpr auto Reverse(T& r) {
	return MakeRange(std::make_reverse_iterator(End(r)), std::make_reverse_iterator(Begin(r)));
}
