#pragma once
#include "ApplyCV.h"
#include "ApplyReference.h"
#include "Type.h"
#include <type_traits>
template <class T, class U> constexpr auto ApplyCVReference(Type<T> t, Type<U> u) {
	return ApplyReference(t, ApplyCV(Type<std::remove_reference_t<T>>{}, u));
}
