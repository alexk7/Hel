#pragma once
#include "ApplyCVReference.h"
#include "BoundTypeIndex.h"
#include "BoundTypes.h"
#include "CartesianProduct.h"
#include "CommonType.h"
#include "Decay.h"
#include "DeclVal.h"
#include "FlattenIndices.h"
#include "MakeArray.h"
#include "MakeList.h"
#include "RemoveRValueReference.h"
#include "ResultType.h"
#include "Size.h"
#include "TConstant.h"
#include "Type.h"
#include "Unique.h"
#include "UnsafeGetAs.h"
class InvokeMultiMethod_t {
	template <class F, class... A, class R, class... V> static R imp(V... v) {
		return static_cast<R>(F{}(UnsafeGetAs(A{}, static_cast<V>(v))...));
	}
public:
	template <class F, class... V> constexpr auto operator()(F, V&&... v) const {
		auto getBoundTypeList = [](auto v) {
			return BoundTypes(Decay(v)) | [&](auto... b) {
				return MakeList(ApplyCVReference(v, b)...);
			};
		};
		auto argTypeListList = CartesianProduct(getBoundTypeList(Type<V&&>{})...);
		auto getResultType = [](auto al) {
			return al | [](auto... a) {
				return ResultType(Type<F>{}, a...);
			};
		};
		using R = decltype(DeclVal(RemoveRValueReference(argTypeListList | [&](auto... al) {
			return Unique(getResultType(al)...) | [](auto... r) {
				return CommonType(r...);
			};
		})));
		auto getImp = [&](auto al) {
			return al | [](auto... a) {
				return TConstant<R(*)(V&&...), &imp<F, decltype(a)...>>{};
			};
		};
		constexpr auto imps = decltype(argTypeListList | [&](auto... al) {
			return MakeArray(getImp(al)...);
		})::Value();
		constexpr auto sizes = decltype(MakeArray(Size(getBoundTypeList(Type<V&&>{}))...))::Value();
		size_t index = FlattenIndices(sizes, BoundTypeIndex(v)...);
		return imps[index](static_cast<V&&>(v)...);
	}
};
constexpr static InvokeMultiMethod_t InvokeMultiMethod{};
