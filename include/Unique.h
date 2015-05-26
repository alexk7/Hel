#pragma once
#include "Constant.h"
#include "FindArg.h"
#include "GetNthArg.h"
#include "MakeIndexList.h"
#include "MakeList.h"
#include "SizeConstant.h"
class Unique_t {
	template <class... X>
	struct Data {
		template <class T> static auto findArg(T) {
			return FindArg(T{}, X{}...);
		}
		size_t indices[sizeof...(X)] = {
			decltype(findArg(X{}))::Value()...
		};
		size_t count = 0;
		constexpr Data() {
			for (size_t i = 0; i < sizeof...(X); ++i) {
				if (indices[i] == i)
					indices[count++] = i;
			}
		}
	};
public:
	template <class... X> auto operator()(Constant<X>...) const {
		constexpr Data<X...> data{};
		return MakeIndexList(SizeConstant<data.count>{}) | [&](auto... i) {
			return MakeList(GetNthArg(SizeConstant<data.indices[i]>{}, X{}...)...);
		};
	}
};
constexpr static Unique_t Unique{};
