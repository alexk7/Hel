#pragma once
#include "Equal.h"
#include "Reverse.h"
#include "StartsWith.h"
constexpr static struct EndsWith_t {
	template <class T, class U, class F = const decltype(Equal)&>
	constexpr auto operator()(const T& t, const U& u, F&& f = Equal) const {
		return StartsWith(Reverse(t), Reverse(u), static_cast<F&&>(f));
	}
} EndsWith{};
