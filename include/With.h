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

/*
template <class... T> class List {
	Closure closure_;
public:
	List(Closure&& closure) : closure_(static_cast<Closure&&>(closure)) {}
	template <class F> constexpr decltype(auto) operator|(F&& f) const& { return closure_(static_cast<F&&>(f)); }
	template <class F> constexpr decltype(auto) operator|(F&& f) && { return static_cast<Closure&&>(closure_)(static_cast<F&&>(f)); }
};
template <class T, class... U> auto MakeList(T&& t, U&&... u) {
	return List<>{[=](auto&& f) {}};
}
*/
