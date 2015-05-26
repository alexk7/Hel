#pragma once
#include "Begin.h"
#include "End.h"
#include "Equal.h"
#include "MakeRange.h"
#include <algorithm>
#include <utility>
constexpr static struct Mismatch_t {
	template <class T, class U, class F = const Equal_t&>
	constexpr auto operator()(const T& t, const U& u, F&& f = Equal) const {
		auto et = End(t);
		auto eu = End(u);
		auto r = std::mismatch(Begin(t), et, Begin(u), eu, static_cast<F&&>(f));
		return std::make_pair(MakeRange(r.first, et), MakeRange(r.second, eu));
	}
} Mismatch{};
