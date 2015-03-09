#pragma once
#include "Invalid.h"
#include "InvokeMultiMethod.h"
#include "PPCAT.h"
#include "Variant.h"
#define DEFINE_MULTIMETHOD(fn)\
	namespace MultiMethodNS {\
		struct PPCAT(fn,_ft) {\
			template <class ...A> constexpr decltype(auto) operator()(A&& ...a) const { return fn(static_cast<A&&>(a)...); }\
		};\
	}\
	namespace VariantNS {\
		template <class ...V> constexpr decltype(auto) fn(V&& ...v) {\
			return InvokeMultiMethod(MultiMethodNS::PPCAT(fn,_ft){}, static_cast<V&&>(v)...);\
		}\
	}\
	namespace InvalidNS {\
		template <class ...V> [[noreturn]] Invalid fn(V&& ...v) {\
			throw std::invalid_argument("Null Variant passed to function.");\
		}\
	}
	
