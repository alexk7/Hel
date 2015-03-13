#pragma once
#include "Equal.h"
#include "Mismatch.h"
constexpr static struct StartsWith_t {
	template <class T, class U, class F = const Equal_t&>
	constexpr auto operator()(const T& t, const U& u, F&& f = Equal) const {
		return empty(Mismatch(t, u, static_cast<F&&>(f)).second);
	}
} StartsWith{};
