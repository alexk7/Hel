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
#include "Type.h"
#include "Unique.h"
#include "UnsafeGetAs.h"
class InvokeMultiMethod_t {
	template <class F, class R, class ...V> struct Imp {
		template <class ...A> struct WithArgs {
			static R value(V... v) {
				return static_cast<R>(F{}(UnsafeGetAs(A{}, static_cast<V>(v))...));
			}
		};
	};
public:
	template <class F, class... V> constexpr auto operator()(F, V&&... v) const {
		auto getBoundTypeList = [](auto v) {
			return BoundTypes(Decay(v)) | [&](auto... b) {
				return MakeList(ApplyCVReference(v, b)...);
			};
		};
		auto boundTypeListList = MakeList(getBoundTypeList(Type<V&&>{})...);
		auto argTypeListList = boundTypeListList | [](auto... bl) {
			return CartesianProduct(bl...);
		};
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
				return VCONSTANT(&Imp<F, R, V&&...>::template WithArgs<decltype(a)...>::value){};
			};
		};
		constexpr auto imps = decltype(argTypeListList | [&](auto... al) {
			return MakeArray(getImp(al)...);
		})::Value();
		constexpr auto sizes = decltype(boundTypeListList | [&](auto... bl) {
			return MakeArray(Size(bl)...);
		})::Value();
		size_t index = FlattenIndices(sizes, BoundTypeIndex(v)...);
		return imps[index](static_cast<V&&>(v)...);
	}
};
constexpr static InvokeMultiMethod_t InvokeMultiMethod{};
