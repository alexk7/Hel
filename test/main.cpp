#include "And.h"
#include "Array.h"
#include "CharConstant.h"
#include "Constant.h"
#include "ConstantFunction.h"
#include "False.h"
#include "MakeArray.h"
#include "MakeUniquePtr.h"
#include "Noop.h"
#include "PPCAT.h"
#include "SizeConstant.h"
#include "Sum.h"
#include "True.h"
#include "Type.h"
#include "TypeName.h"
#include "Unit.h"
#include "UnitList.h"
#include <algorithm>
#include <array>
#include <assert.h>
#include <iostream>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <utility>

struct Void : Unit<Void> {};
template <class T, class ...U> constexpr auto Front(UnitList<T, U...>) { return T{}; }
template <class T, class ...U> constexpr auto Front(UnitList<>) { return Void{}; }
template <class T, class ...U> constexpr auto Tail(UnitList<T, U...>) { return UnitList<U...>{}; }

template <class ...T> constexpr auto ToArray(UnitList<Constant<T>...>) {
	return MakeArray(T::value...);
}
template <class ...T> constexpr auto ToArray(UnitList<T...>) {
	return MakeArray(ToArray(T{})...);
}

template <class T> T DeclVal(Type<T>);
template <class T> constexpr auto Decay(Type<T>) { return Type<std::decay_t<T>>{}; }
template <class T> constexpr auto RemoveRValueReference(Type<T&&>) { return Type<T>{}; }
template <class T> constexpr auto RemoveRValueReference(Type<T>) { return Type<T>{}; }

constexpr static struct If_t : ConstantFunction<If_t> {
	template <class Then, class Else> constexpr auto operator()(BoolConstant<true>, Then t, Else) const { return t; }
	template <class Then, class Else> constexpr auto operator()(BoolConstant<false>, Then, Else e) const { return e; }
	template <class Cond, class Then, class Else> constexpr auto operator()(Constant<Cond>, Then t, Else e) const {
		return (*this)(BoolConstant<static_cast<bool>(Cond::value)>{}, t, e);
	}
} If{};

template <class T, char c> constexpr auto Parse(Type<T>, UnitList<CharConstant<c>>) {
	return TypedConstant<T, c - '0'>{};
}
template <char ...c> constexpr auto operator""_z() { return Parse(Type<size_t>{}, UnitList<CharConstant<c>...>{}); }

template <class T, class U> constexpr auto operator+(Constant<T> a, Constant<U> b) {
	return Sum(a, b);
}

template <class ...T> constexpr auto Size(UnitList<T...>) { return SizeConstant<sizeof...(T)>{}; }
template <class ...T> constexpr auto MakeList(Unit<T>...) { return UnitList<T...>{}; }
template <class ...T, class F> constexpr decltype(auto) Unpack(UnitList<T...>, F f) { return f(T{}...); }

template <class T> constexpr auto CommonType(Type<T> t) { return t; }
template <class T, class U> constexpr auto CommonType(Type<T>, Type<U>) {
	return Type<decltype(false ? std::declval<T>() : std::declval<U>())>{};
}
template <class T, class U, class ...V> constexpr auto CommonType(Type<T> t, Type<U> u, Type<V> ...v) {
	return CommonType(CommonType(t, u), v...);
}

template <class F, class ...A> constexpr auto ResultType(Type<F> f, UnitList<Type<A>...>) {
	return Type<decltype(DeclVal(f)(DeclVal(Type<A>{})...))>{};
}

template <class T, class U> constexpr auto ApplyCV(Type<T>, Type<U>) { return Type<U>{}; }
template <class T, class U> constexpr auto ApplyCV(Type<const T>, Type<U>) { return Type<const U>{}; }
template <class T, class U> constexpr auto ApplyCV(Type<volatile T>, Type<U>) { return Type<volatile U>{}; }
template <class T, class U> constexpr auto ApplyCV(Type<const volatile T>, Type<U>) { return Type<const volatile U>{}; }

template <class T, class U> constexpr auto ApplyReference(Type<T>, Type<U>) { return Type<U>{}; }
template <class T, class U> constexpr auto ApplyReference(Type<T&>, Type<U>) { return Type<U&>{}; }
template <class T, class U> constexpr auto ApplyReference(Type<T&&>, Type<U>) { return Type<U&&>{}; }

template <class T, class U> constexpr auto ApplyCVReference(Type<T> t, Type<U> u) {
	return ApplyReference(t, ApplyCV(Type<std::remove_reference_t<T>>{}, u));
}

template <class T> constexpr auto FindType(T, UnitList<>) {
	return 0_z;
}
template <class T, class ...U> constexpr auto FindType(T, UnitList<T, U...>) {
	return 0_z;
}
template <class T, class U, class ...V> constexpr auto FindType(T, UnitList<U, V...>) {
	return FindType(T{}, UnitList<V...>{}) + 1_z;
}

template <class ...T, class ...U> constexpr auto Concatenate(UnitList<T...>, UnitList<U...>) {
	return UnitList<T..., U...>{};
}

template <class T, class ...U> constexpr auto Append(Unit<T>, UnitList<U...>) { return UnitList<U..., T>{}; }
template <class T, class ...U> constexpr auto Prepend(Unit<T>, UnitList<U...>) { return UnitList<T, U...>{}; }

template <class T, class ...L> constexpr auto PrependToEach(Unit<T>, UnitList<L...>) {
	return MakeList(Prepend(T{}, L{})...);
}

template <class ...T> constexpr auto CartesianProduct(UnitList<T...>) {
	return UnitList<UnitList<T...>>{};
}
template <class T, class L, class ...M> constexpr auto CartesianProduct(UnitList<T>, L, M...) {
	return PrependToEach(T{}, CartesianProduct(L{}, M{}...));
}
template <class T, class U, class ...W, class ...L> constexpr auto CartesianProduct(UnitList<T, U, W...>, L...) {
	return Concatenate(CartesianProduct(UnitList<T>{}, L{}...),
					   CartesianProduct(UnitList<U, W...>{}, L{}...));
}

template <class F> class Recursive {
	F f_;
public:
	Recursive(const F& f) : f_(f) {}
	Recursive(F&& f) : f_(static_cast<F&&>(f)) {}
	template <class ...A> constexpr auto operator()(A&& ...a) const { return f_(*this, static_cast<A&&>(a)...); }
};

template <class F> constexpr auto MakeRecursive(F&& f) { return Recursive<std::decay_t<F>>{static_cast<F&&>(f)}; }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace VariantNS {

class VariantBase {
protected:
	template <class T> static void CastAndDelete(void* _p) { delete static_cast<T*>(_p); }
	static void Noop(void*) {}
	struct Data {
		size_t tag = 0;
		void* buffer = nullptr;
	};
	Data data_;
	constexpr VariantBase() noexcept = default;
	VariantBase(VariantBase&& _u) noexcept {
		data_ = _u.data_;
		_u.data_ = {};
	}
};

}

template <class ...Types>
class Variant : VariantNS::VariantBase {
	constexpr static auto typeList_ = MakeList(Type<Types>{}...);
	constexpr static auto typeCount_ = Size(typeList_);
	template <class T> constexpr static auto Tag(Type<T> t) { return FindType(t, UnitList<Type<Types>...>{}) + 1_z; }
	template <class T> constexpr static auto ValidType(Type<T> t) { return Variant::Tag(t) <= typeCount_; }
	template <class To, class From> static constexpr auto ValidGet(Type<To> to, Type<From> from) {
		constexpr auto toval = Decay(Type<To>{});
		constexpr auto fromval = Decay(Type<From>{});
		static_assert(Type<Variant>{} == fromval, "Calling GetAs with wrong Variant type!");
		return And(Variant::ValidType(toval), ApplyCVReference(from, toval) == to);
	}
public:
	constexpr Variant() noexcept {}
	constexpr Variant(std::nullptr_t) noexcept {}
	Variant(Variant&&) noexcept = default;
	template <class T, class U, class = std::enable_if_t<ValidType(Type<U>{})>>
	Variant(T&& t, Type<U> u) { data_ = { Tag(u), new U(static_cast<T&&>(t)) }; }
	template <class T> Variant(T&& t) : Variant(static_cast<T&&>(t), Decay(Type<T>{})) {}
	~Variant() {
		using FP = void (*)(void*);
		static const FP kFPs[] = { Noop, CastAndDelete<Types>... };
		kFPs[data_.tag](data_.buffer);
	}
	Variant& operator=(Variant _u) { swap(*this, _u); return *this; }
	friend void swap(Variant& _a, Variant& _b) { std::swap(_a.data_, _b.data_); }
	template <class T, class U>
	friend std::enable_if_t<Variant::ValidGet(Type<T>{}, Type<U&&>{}), T> UnsafeGetAs(Type<T>, U&& u) {
		return static_cast<T>(*static_cast<std::remove_reference_t<T>*>(u.data_.buffer));
	}
	template <class T, class U>
	friend std::enable_if_t<Variant::ValidGet(Type<T>{}, Type<U&&>{}), T> GetAs(Type<T>, U&& u) {
		if (u.data_.tag != Tag(Decay(Type<T>{})))
			throw std::bad_cast{};
		return static_cast<T>(*static_cast<std::remove_reference_t<T>*>(u.data_.buffer));
	}
	friend size_t BoundTypeIndex(const Variant& v) { return v.data_.tag; }
};

template <class ...T> constexpr auto BoundTypes(Type<Variant<T...>>) { return UnitList<Type<T>...>{}; }

template <class T> constexpr size_t BoundTypeIndex(const T&) { return 0; }
template <class T> constexpr auto BoundTypes(Type<T>) { return UnitList<Type<T>>{}; }

template <class T, class U> std::enable_if_t<Type<T>{} == Type<U&&>{}, T> UnsafeGetAs(Type<T>, U&& u) {
	return static_cast<T>(u);
}

template <class T, class U> std::enable_if_t<Type<T>{} == Type<U&&>{}, T> GetAs(Type<T>, U&& u) {
	return static_cast<T>(u);
}

template <class T, size_t s> decltype(auto) Subscript(const Array<T, s>& a, size_t i) { return a.value[i]; }
template <class T, size_t s, class ...S> decltype(auto) Subscript(const Array<T, s>& a, size_t i1, size_t i2, S ...i3) {
	return Subscript(Subscript(a, i1), i2, i3...);
}

/*
constexpr static struct While_t {
	struct Step {
		template <class P, class S, class F> constexpr decltype(auto) operator()(P&& p, S&& s, F&& f) const {
			return While_t{}(static_cast<P&&>(p), f(static_cast<S&&>(s)), static_cast<F&&>(f));
		}
	};
	struct End {
		template <class P, class S, class F> constexpr decltype(auto) operator()(P&&, S&& s, F&&) const {
			return static_cast<S&&>(s);
		}
	};
	template <class P, class S, class F> constexpr decltype(auto) operator()(P&& p, S&& s, F&& f) const {
		return If(p(s), Step{}, End{})(static_cast<P&&>(p), static_cast<S&&>(s), static_cast<F&&>(f));
	}
} While{};
*/

constexpr static struct Identity_t : ConstantFunction<Identity_t> {
	using ConstantFunction::operator();
	template <class T> constexpr T operator()(T&& t) const { return static_cast<T&&>(t); }
} Identity{};

template <class F> struct UnitLambda {
	static_assert(std::is_empty<F>{}, "T must be stateless.");
	template <class ...A> constexpr decltype(auto) operator()(A&& ...a) const {
		return (*(F*)(0))(Identity, static_cast<A&&>(a)...);
	}
};
struct UnitLambdaHack {
	template <class T> T* operator+(T) const { return nullptr; }
	template <class T> constexpr auto operator+=(T*) const { return UnitLambda<T>{}; }
};
#define UNIT_LAMBDA(...) UnitLambdaHack{} += true ? nullptr : UnitLambdaHack{} + [](auto delay, ##__VA_ARGS__)
template <class F, class R, class ...A> constexpr auto Cast(Type<R(*)(A...)>, UnitLambda<F>) {
	struct Thunk {
		static R value(A... a) { return UnitLambda<F>{}(static_cast<A>(a)...); }
	};
	return Constant<Thunk>{};
}

template <class F> struct DeclValLambda {
	template <class ...A> constexpr auto operator()(A&& ...a) const
		-> decltype(std::declval<F>()(Identity, static_cast<A&&>(a)...));
};
struct DeclValLambdaHack {
	template <class T> T* operator+(T) const { return nullptr; }
	template <class T> constexpr auto operator+=(T*) const { return UnitLambda<T>{}; }
};
#define DECLVAL_LAMBDA(...) DeclValLambdaHack{} += true ? nullptr : DeclValLambdaHack{} + [&](auto delay, ##__VA_ARGS__)

#define DECLVAL_IF(Cond, Then, Else) If(Cond, DECLVAL_LAMBDA() { return Then; }, DECLVAL_LAMBDA() { return Else; })

template <class F, class ...V> static auto InvokeMultiMethod(V&& ...v) {
	constexpr static auto all_ = CartesianProduct(BoundTypes(Decay(Type<V>{}))...);
	using R = decltype(DeclVal(Unpack(all_, [](auto ...al) {
		return RemoveRValueReference(CommonType(Unpack(al, [](auto ...a) {
			return ResultType(Type<F>{}, MakeList(ApplyCVReference(Type<V&&>{}, a)...));
		})...));
	})));
	auto bll = MakeList(BoundTypes(Decay(Type<V>{}))...);
	auto getImps = MakeRecursive([](auto self, auto boundArgListList, auto argList) {
		static auto getImps = self;
		constexpr static auto al = decltype(argList){};
		return If(Size(boundArgListList) == 0_z, DECLVAL_LAMBDA() {
			return Unpack(delay(al), [](auto ...a) {
				return Cast(Type<R(*)(V&&...)>{}, If(And((a != Type<void>{})...), UNIT_LAMBDA(V&& ...v) -> R {
					return F{}(UnsafeGetAs(ApplyCVReference(Type<V&&>{}, delay(decltype(a){})), static_cast<V&&>(v))...);
				}, UNIT_LAMBDA(V&&...) -> R {
					throw std::invalid_argument("Null Variant passed to function.");
				}));
			});
		}, DECLVAL_LAMBDA() {
			constexpr static auto bll = delay(decltype(boundArgListList){});
			constexpr static auto tail = Tail(bll);
			return Unpack(Front(bll), DECLVAL_LAMBDA(auto... b) {
				return MakeArray(getImps(tail, Append(Type<void>{}, al)),
								 getImps(tail, Append(b, al))...);
			});
		})();
	});
	constexpr static auto imps = decltype(getImps(bll, UnitList<>{}))::value;
	return Subscript(imps, BoundTypeIndex(v)...)(static_cast<V&&>(v)...);
}

#if 1
#define DEFINE_FUNCTION(fn)\
	namespace Hel {\
		struct PPCAT(fn,_ft) {\
			template <class ...A> constexpr decltype(auto) operator()(A&& ...a) const { return fn(static_cast<A&&>(a)...); }\
		};\
	}
#define DEFINE_MULTIMETHOD(fn)\
	DEFINE_FUNCTION(fn)\
	namespace VariantNS {\
		template <class ...V> constexpr decltype(auto) fn(V&& ...v) {\
			return InvokeMultiMethod<Hel::PPCAT(fn,_ft)>(static_cast<V&&>(v)...);\
		}\
	}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define DTOR_TEST_IMPL
//#define DTOR_TEST_IMPL cout << __PRETTY_FUNCTION__ << endl;

struct Circle { ~Circle() { DTOR_TEST_IMPL } };
struct Rectangle { ~Rectangle() { DTOR_TEST_IMPL } };
struct Triangle { ~Triangle() { DTOR_TEST_IMPL } };
struct S1 { ~S1() { DTOR_TEST_IMPL } };
struct S2 { ~S2() { DTOR_TEST_IMPL } };
struct S3 { ~S3() { DTOR_TEST_IMPL } };
struct S4 { ~S4() { DTOR_TEST_IMPL } };
struct S5 { ~S5() { DTOR_TEST_IMPL } };
struct S6 { ~S6() { DTOR_TEST_IMPL } };
struct S7 { ~S7() { DTOR_TEST_IMPL } };
struct S8 { ~S8() { DTOR_TEST_IMPL } };
struct S9 { ~S9() { DTOR_TEST_IMPL } };
struct S10 { ~S10() { DTOR_TEST_IMPL } };

inline bool Intersect(Circle, Rectangle) {
	puts(__PRETTY_FUNCTION__);
	return false;
}

inline bool Intersect(const Rectangle&, Circle) {
	puts(__PRETTY_FUNCTION__);
	return false;
}

inline bool Intersect(Circle, Circle) {
	puts(__PRETTY_FUNCTION__);
	return true;
}

inline bool Intersect(Rectangle, Rectangle) {
	puts(__PRETTY_FUNCTION__);
	return true;
}

inline bool Intersect(Rectangle&&, Circle) {
	puts(__PRETTY_FUNCTION__);
	return false;
}

inline void Print(Circle) {
	puts(__PRETTY_FUNCTION__);
}

inline void Print(Rectangle) {
	puts(__PRETTY_FUNCTION__);
}

inline void Print(Triangle) {
	puts(__PRETTY_FUNCTION__);
}

inline int Index(const Circle&) {
	puts(__PRETTY_FUNCTION__);
	return -1;
}

inline unsigned Index(Circle&&) {
	puts(__PRETTY_FUNCTION__);
	return 1;
}

inline long Index(Rectangle) {
	puts(__PRETTY_FUNCTION__);
	return 2;
}

inline long long Index(Triangle) {
	puts(__PRETTY_FUNCTION__);
	return 0;
}

inline Circle Copy(Circle c) {
	puts(__PRETTY_FUNCTION__);
	return c;
}

inline Rectangle Copy(Rectangle r) {
	puts(__PRETTY_FUNCTION__);
	return r;
}

inline Triangle Copy(Triangle t) {
	puts(__PRETTY_FUNCTION__);
	return t;
}

#if 1
DEFINE_MULTIMETHOD(Print)
DEFINE_MULTIMETHOD(Index)
DEFINE_MULTIMETHOD(Intersect)
DEFINE_MULTIMETHOD(Copy)
#endif

#if 0
#define fn Intersect
namespace Fn {
	struct PPCAT(fn,_t) {
		template <class ...A> decltype(auto) operator()(A&& ...a) const { return fn(forward<A>(a)...); }
	};
}
namespace VariantNS {
	template <class ...V> decltype(auto) fn(V&& ...v) {
		return MultiMethodCall<Fn::PPCAT(fn,_t), V...>::Dispatch(forward<V>(v)...);
	}
}
#endif

using namespace std;
using Shape3 = Variant<Circle, Rectangle, S1, S2, S3, S4, S5, S6, S7, S8, S9, S10, Triangle>;

int main()
{
#if 1
	using Shape2 = Variant<Circle, Rectangle>;
	Shape2 v, v2;
	v = Circle{};
	v2 = Rectangle{};
	cout << Intersect(v, v) << endl;
	cout << Intersect(v, v2) << endl;
	cout << Intersect(v2, v2) << endl;
	cout << Intersect(v2, v) << endl;
	cout << Intersect(move(v2), v) << endl;
#endif

#if 1
	Circle c;
	Rectangle r;

	{
		using Shape = Variant<Triangle, Circle, Rectangle>;
		Shape s{c};
		Print(s);

		Shape s2;
		s2 = c;
		Shape s3{r};
		Shape s4{nullptr};
		//Print(s4);
		//Shape s5{"allo"};
		//Shape s6{0}; //delegating ctor disable 0 -> nullptr conversion
		//Shape s6_1{1};
		Shape s7{Triangle{}};
		Shape s8{move(s)};
		//Shape s8_1{double{}};
		Shape s9;
		char c = getchar();
		if (c == 'c')
			s9 = Circle{};
		else if (c == 'r')
			s9 = Rectangle{};
		else if (c == 't')
			s9 = Triangle{};

		try {
			Print(s9);
			cout << Index(s9) << endl;
			cout << Index(move(s9)) << endl;
		} catch (const exception& e) {
			puts(e.what());
		}

		//cout << "----------" << endl;
		//s7 = move(s3);

		//cout << "----------" << endl;
	}

	cout << "----------" << endl;
#endif
}
