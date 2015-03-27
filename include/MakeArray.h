#pragma once
#include "Array.h"
#include <type_traits>
constexpr static struct MakeArray_t : ConstantFunction<MakeArray_t> {
	template <class T, class... U> constexpr static auto invoke(T&& t, U&&... u) {
		return Array<std::decay_t<T>, 1 + sizeof...(U)>{{ static_cast<T&&>(t), static_cast<U&&>(u)... }};
	}
} MakeArray{};
