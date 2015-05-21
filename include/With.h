#pragma once
template <class Closure> class ArgListProxy {
	Closure closure_;
public:
	ArgListProxy(Closure closure) : closure_(closure) {}
	template <class F> constexpr decltype(auto) operator|(F&& f) const&& { return closure_(static_cast<F&&>(f)); }
};
template <class T> ArgListProxy<T> MakeArgListProxy(T t) { return ArgListProxy<T>{t}; }
static auto With = [](auto&&... a) {
	return MakeArgListProxy([&](auto&& f) { return static_cast<decltype(f)>(f)(static_cast<decltype(a)>(a)...); });
};
