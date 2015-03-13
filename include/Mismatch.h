#pragma once
#include "Equal.h"
#include "MakeRange.h"
#include <utility>
constexpr static struct Mismatch_t {
	template <class T, class U, class F = const Equal_t&>
	constexpr auto operator()(const T& t, const U& u, F&& f = Equal) const {
		auto r = std::mismatch(begin(t), end(t), begin(u), end(u), static_cast<F&&>(f));
		return std::make_pair(MakeRange(r.first, end(t)), MakeRange(r.second, end(u)));
	}
} Mismatch{};
