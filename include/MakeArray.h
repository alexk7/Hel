#pragma once
#include "Array.h"
#include <type_traits>
constexpr static struct MakeArray_t {
	template <class T, class... U> constexpr auto operator()(T&& t, U&&... u) const {
		return Array<std::decay_t<T>, 1 + sizeof...(U)>{{ static_cast<T&&>(t), static_cast<U&&>(u)... }};
	}
} MakeArray{};
