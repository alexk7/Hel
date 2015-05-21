#pragma once
#include "SizeConstant.h"
#include "SizeConstantList.h"
#include "_z.h"
#include <MakeIndexList.g>
constexpr static inline SizeConstantList<> MakeIndexList(SizeConstant<0>) { return {}; }
//*
template <size_t N> constexpr static auto MakeIndexList(SizeConstant<N> n) {
	auto h = n / 2_z;
	auto al = MakeIndexList(h);
	auto bl = MakeIndexList(n - h);
	return al | [&](auto... a) {
		return bl | [&](auto... b) {
			return MakeList(a..., (h + b)...);
		};
	};
}
//*/
