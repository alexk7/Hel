#pragma once
#include "Array.h"
#include "ConstantFunction.h"
#include <type_traits>
struct MakeArray_t {
	template <class T, class... U> constexpr static auto invoke(T&& t, U&&... u) {
		return Array<std::decay_t<T>, 1 + sizeof...(U)>{{ static_cast<T&&>(t), static_cast<U&&>(u)... }};
	}
};
constexpr static ConstantFunction<MakeArray_t> MakeArray{};
