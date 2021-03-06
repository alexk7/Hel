#pragma once
#include "Empty.h"
#include "Equal.h"
#include "Mismatch.h"
constexpr static struct StartsWith_t {
	template <class T, class U, class F = const decltype(Equal)&>
	constexpr auto operator()(const T& t, const U& u, F&& f = Equal) const {
		return Empty(Mismatch(t, u, static_cast<F&&>(f)).second);
	}
} StartsWith{};
