#pragma once
#include "ConstantFunction.h"
#include "False.h"
#include "Noop.h"
struct Or_t {
	constexpr static auto invoke() { return False; }
	template <class T, class ...U> constexpr static auto invoke(const T& t, const U&... u) {
		bool b = static_cast<bool>(t);
		(void)(bool[]){(b = b || u)...};
		return b;
	}
}
constexpr static ConstantFunction<Or_t> Or{};
