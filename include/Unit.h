#pragma once
template <class T> struct Unit {};
template <class A, class B> constexpr auto operator==(Unit<A>, Unit<B>) { return A{} == B{}; }
template <class A, class B> constexpr auto operator!=(Unit<A>, Unit<B>) { return !(A{} == B{}); }
template <class A, class B> constexpr auto operator<(Unit<A>, Unit<B>) { return A{} < B{}; }
template <class A, class B> constexpr auto operator>(Unit<A>, Unit<B>) { return B{} < A{}; }
template <class A, class B> constexpr auto operator<=(Unit<A>, Unit<B>) { return !(B{} < A{}); }
template <class A, class B> constexpr auto operator>=(Unit<A>, Unit<B>) { return !(A{} < B{}); }
