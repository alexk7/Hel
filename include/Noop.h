#pragma once
constexpr static struct Noop_t {
	template <class ...T> constexpr void operator()(const T&...) const {}
} Noop{};
