#pragma once
#include <type_traits>
template <class D> struct Constant {
	template <class T> constexpr operator T() const { return D::Value(); }
};

template <class T, T t> struct TConstant : Constant<TConstant<T, t>> {
	constexpr static T Value() { return t; }
};
#define TCONSTANT(...) TConstant<std::remove_const_t<decltype(__VA_ARGS__)>, __VA_ARGS__>

template <size_t p> struct SfinaePriority : SfinaePriority<p - 1> {};
template <> struct SfinaePriority<0> {};

template <class F> class ConstantFunction {
	template <class S> static void VerifyConstant(const Constant<S>& c) {}
	template <class... T> constexpr static auto SfinaeHack(SfinaePriority<2>, const T&... t)
		-> TCONSTANT(F::invoke(decltype(VerifyConstant(t), T{})::Value()...)) {
		return {};
	}
	template <class... T> struct Result : Constant<Result<T...>> {
		constexpr static auto value = F::invoke(T::Value()...);
		constexpr static auto Value() { return value; }
	};
	template <class... T> constexpr static auto SfinaeHack(SfinaePriority<1>, const T&... t)
		-> Result<decltype(VerifyConstant(t), T{})...> {
		return {};
	}
	template <class... T> constexpr static auto SfinaeHack(SfinaePriority<0>, const T&... t) {
		return F::invoke(t...);
	}
public:
	template <class T, class ...U> constexpr auto operator()(T t, U ...u) const {
		return SfinaeHack(SfinaePriority<2>{}, t, u...);
	}
};

struct Not_t {
	template <class T> constexpr static auto invoke(T t) { return !t; }
};
constexpr static ConstantFunction<Not_t> Not{};
template <class T> constexpr auto operator!(Constant<T>) { return Not(T{}); }

struct Equal_t {
	template <class A, class B> constexpr static auto invoke(const A& a, const B& b) { return a == b; }
};
constexpr static ConstantFunction<Equal_t> Equal{};
template <class A, class B> constexpr auto operator==(Constant<A>, Constant<B>) { return Equal(A{}, B{}); }

struct Less_t {
	template <class A, class B> constexpr static auto invoke(const A& a, const B& b) { return a < b; }
};
constexpr static ConstantFunction<Less_t> Less{};
template <class A, class B> constexpr auto operator<(Constant<A>, Constant<B>) { return Less(A{}, B{}); }

struct Or_t {
	template <class A, class B> constexpr static auto invoke(const A& a, const B& b) { return a || b; }
};
constexpr static ConstantFunction<Or_t> Or{};
template <class A, class B> constexpr auto operator||(Constant<A>, Constant<B>) { return Or(A{}, B{}); }

struct And_t {
	template <class A, class B> constexpr static auto invoke(const A& a, const B& b) { return a && b; }
};
constexpr static ConstantFunction<And_t> And{};
template <class A, class B> constexpr auto operator&&(Constant<A>, Constant<B>) { return And(A{}, B{}); }

struct Neg_t {
	template <class T> constexpr static auto invoke(T t) { return -t; }
};
constexpr static ConstantFunction<Neg_t> Neg{};
template <class T> constexpr auto operator-(Constant<T>) { return Neg(T{}); }

struct Add_t {
	template <class A, class B> constexpr static auto invoke(const A& a, const B& b) { return a + b; }
};
constexpr static ConstantFunction<Add_t> Add{};
template <class A, class B> constexpr auto operator+(Constant<A>, Constant<B>) { return Add(A{}, B{}); }

struct Sub_t {
	template <class A, class B> constexpr static auto invoke(const A& a, const B& b) { return a - b; }
};
constexpr static ConstantFunction<Sub_t> Sub{};
template <class A, class B> constexpr auto operator-(Constant<A>, Constant<B>) { return Sub(A{}, B{}); }

struct Mul_t {
	template <class A, class B> constexpr static auto invoke(const A& a, const B& b) { return a - b; }
};
constexpr static ConstantFunction<Mul_t> Mul{};
template <class A, class B> constexpr auto operator*(Constant<A>, Constant<B>) { return Mul(A{}, B{}); }

struct Div_t {
	template <class A, class B> constexpr static auto invoke(const A& a, const B& b) { return a / b; }
};
constexpr static ConstantFunction<Div_t> Div{};
template <class A, class B> constexpr auto operator/(Constant<A>, Constant<B>) { return Div(A{}, B{}); }

template <class A, class B> constexpr auto operator!=(Constant<A>, Constant<B>) { return !(A{} == B{}); }
template <class A, class B> constexpr auto operator>(Constant<A>, Constant<B>) { return B{} < A{}; }
template <class A, class B> constexpr auto operator<=(Constant<A>, Constant<B>) { return !(B{} < A{}); }
template <class A, class B> constexpr auto operator>=(Constant<A>, Constant<B>) { return !(A{} < B{}); }
