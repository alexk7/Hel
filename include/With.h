#pragma once
constexpr static struct With_t {
	template <class T, class F>
	constexpr auto operator()(T&& t, F&& f) const {
		return static_cast<F&&>(f)(static_cast<T&&>(t));
	}
} With{};
