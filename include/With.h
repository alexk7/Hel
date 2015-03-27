#pragma once

template <class Closure> class Continuation {
	Closure closure_;
public:
	Continuation(Closure closure) : closure_(closure) {}
	template <class F> constexpr decltype(auto) operator|(F&& f) const&& { return closure_(static_cast<F&&>(f)); }
};

template <class Closure>
Continuation<Closure> MakeContinuation(Closure closure) { return Continuation<Closure>{closure}; }

constexpr static struct With_t {
	template <class... T> constexpr decltype(auto) operator()(T&&... t) const {
		return MakeContinuation([&](auto&& f) { return static_cast<decltype(f)>(f)(static_cast<T&&>(t)...); });
	}
} With{};
