#pragma once
#include "Array.h"
#include "MakeIndexList.h"
#include "MakeList.h"
#include "Multiply.h"
#include "Size.h"
#include "SizeConstant.h"
class CartesianProduct_t {
	template <size_t... s> constexpr static auto MakeIndexArray(size_t i) {
		constexpr size_t sizes[] = { s... };
		constexpr size_t n = sizeof...(s);
		Array<size_t, n> result{};
		for (size_t j = n; j--;) {
			result[j] = i % sizes[j];
			i /= sizes[j];
		}
		return result;
	}
public:
	template <class... L> auto operator()(L... l) const {
		auto makePermutation = [&l...](auto... i) {
			return MakeList(l[i]...);
		};
		constexpr static auto elementIndexList = MakeIndexList(SizeConstant<sizeof...(L)>{});
		auto makeElement = [&](auto i) {
			constexpr static auto indexArray = MakeIndexArray<decltype(Size(l))::Value()...>(i);
			return elementIndexList | [&](auto... j) {
				return makePermutation(SizeConstant<indexArray[j]>{}...);
			};
		};
		return MakeIndexList(Multiply(Size(l)...)) | [&](auto... i) {
			return MakeList(makeElement(i)...);
		};
	}
};
constexpr static CartesianProduct_t CartesianProduct{};
