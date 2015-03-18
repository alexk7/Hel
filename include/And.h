#pragma once
#include "ConstantFunction.h"
#include "Noop.h"
#include "True.h"
constexpr struct And_t : ConstantFunction<And_t> {
	using ConstantFunction::operator();
	constexpr auto operator()() const { return True; }
	template <class T, class ...U> constexpr auto operator()(const T& t, const U&... u) const {
		bool b = static_cast<bool>(t);
		Noop((b = b && u)...);
		return b;
	}
} And{};
