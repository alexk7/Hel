#pragma once
#include "ConstantFunction.h"
#include "False.h"
#include "Noop.h"
constexpr struct Or_t : ConstantFunction<Or_t> {
	using ConstantFunction::operator();
	constexpr auto operator()() const { return False; }
	template <class T, class ...U> constexpr auto operator()(const T& t, const U&... u) const {
		bool b = static_cast<bool>(t);
		Noop((b = b || u)...);
		return b;
	}
} Or{};
