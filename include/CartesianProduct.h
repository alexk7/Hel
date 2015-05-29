#pragma once
#include "Array.h"
#include "MakeIndexList.h"
#include "MakeList.h"
#include "Multiply.h"
#include "Size.h"
#include "SizeConstant.h"
class CartesianProduct_t {
	template <size_t... s> constexpr static auto MakeIndexArray(size_t i, const SizeConstant<s>&...) {
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
		auto makeElement = [&l...](auto... i) {
			return MakeList(l[i]...);
		};
		constexpr auto elementIndexList = MakeIndexList(ArgCount(l...));
		auto makeNthElement = [&](auto i) {
			constexpr auto indexArray = MakeIndexArray(i, Size(l)...);
			return elementIndexList | [&](auto... j) {
				return makeElement(SizeConstant<indexArray[j]>{}...);
			};
		};
		return MakeIndexList(Multiply(Size(l)...)) | [&](auto... i) {
			return MakeList(makeNthElement(i)...);
		};
	}
};
constexpr static CartesianProduct_t CartesianProduct{};
