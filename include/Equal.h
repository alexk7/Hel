#pragma once
constexpr static struct Equal_t {
	template <class T, class U> constexpr auto operator()(const T& t, const U& u) const {
		return t == u;
	}
} Equal{};
