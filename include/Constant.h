#pragma once
#include "BoolConstant.h"
#include "NonTypeTemplateParameter.h"
#include "TypedConstant.h"
#include "Unit.h"
#include <type_traits>
template <class V> struct Constant : Unit<Constant<V>> {
	using type = decltype(V::value);
	constexpr static type value = V::value;
	constexpr operator type() const { return value; }
};
template <class T, T t> using TypedConstant = Constant<NonTypeTemplateParameter<T, t>>;
constexpr static class GetConstant_t {
	template <class V> static TypedConstant<std::remove_const_t<decltype(V::value)>, V::value> sfinae(int);
	template <class V> static TypedConstant<const decltype(V::value)&, V::value> sfinae(long);
public:
	template <class V> constexpr auto operator()(V) const { return decltype(sfinae<V>(0)){}; }
} GetConstant{};
template <bool b> using BoolConstant = TypedConstant<bool, b>;
template <class T> constexpr auto operator!(Constant<T>) { return BoolConstant<!T::value>{}; }
template <class A, class B> constexpr auto operator==(Constant<A>, Constant<B>) {
	return BoolConstant<A::value == B::value>{};
}
template <class A, class B> constexpr auto operator<(Constant<A>, Constant<B>) {
	return BoolConstant<A::value < B::value>{};
}
template <class A, class B> constexpr auto operator||(Constant<A>, Constant<B>) {
	return BoolConstant<A::value || B::value>{};
}
