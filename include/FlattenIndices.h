#pragma once
#include "Array.h"
#include "ConstantFunction.h"
struct FlattenIndices_t {
	template <size_t count, class... S>
	constexpr static size_t invoke(const Array<size_t, count> sizes, S... index) {
		static_assert(count == sizeof...(S), "");
		size_t indices[] = { index... };
		size_t r = 0;
		size_t i = count;
		size_t m = 1;
		while (i--) {
			r += m * indices[i];
			m *= sizes[i];
		}
		return r;
	}
};
constexpr static ConstantFunction<FlattenIndices_t> FlattenIndices{};
