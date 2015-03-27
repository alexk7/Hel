#pragma once
#include "BoolConstant.h"
#include "NonTypeTemplateParameter.h"
#include "TypedConstant.h"
#include "Unit.h"
#include <type_traits>
template <class V> struct Constant : Unit<Constant<V>> {
	using type = std::decay_t<decltype(V::value)>;
	constexpr static type value = V::value;
	constexpr operator type() const { return value; }
};
template <class T, T t> using TypedConstant = Constant<NonTypeTemplateParameter<T, t>>;
template <class R> constexpr static TypedConstant<decltype(R::value), R::value> GetConstant(R, R) { return {}; }
template <class R> constexpr static Constant<R> GetConstant(R, ...) { return {}; }
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
