#pragma once
#include <memory>
constexpr static struct MakeUniquePtr_t {
	template <class T, class D> constexpr auto operator()(T* p, D d) const { return std::unique_ptr<T, D>{p, d}; }
} MakeUniquePtr{};
