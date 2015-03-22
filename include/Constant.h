#pragma once
#include "NonTypeTemplateParameter.h"
#include "Unit.h"
#include <type_traits>
template <class V> struct Constant;
template <class T, T t> using TypedConstant = Constant<NonTypeTemplateParameter<T, t>>;
template <bool b> using BoolConstant = TypedConstant<bool, b>;
template <class V> struct Constant : Unit<Constant<V>> {
	using type = std::decay_t<decltype(V::value)>;
	constexpr static type value = V::value;
	constexpr operator type() const { return value; }
	constexpr operator BoolConstant<!!value>() const { return {}; }
};
template <class T> constexpr auto operator!(Constant<T>) { return BoolConstant<!T::value>{}; }
template <class A, class B> constexpr auto operator==(Constant<A>, Constant<B>) {
	return BoolConstant<A::value == B::value>{};
}
template <class A, class B> constexpr auto operator<(Constant<A>, Constant<B>) {
	return BoolConstant<A::value < B::value>{};
}
