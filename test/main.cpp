#include <cassert>
#include <iostream>
#include <utility>
#include <tuple>
#include <type_traits>
#include <cxxabi.h>
#include <array>
#include <stdexcept>
#include <typeinfo>
using namespace std;

#define PPCAT_NX(A, B) A ## B
#define PPCAT(A, B) PPCAT_NX(A, B)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Types

template <class T> struct Unit {};
template <class ...T> struct UnitList : Unit<UnitList<T...>> {};
template <class T> struct Type : Unit<Type<T>> {};
template <class T, T t> struct Constant : Unit<Constant<T, t>> {
	constexpr operator T() const { return t; }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Objects

constexpr auto True = Constant<bool, true>{};
constexpr auto False = Constant<bool, false>{};

constexpr struct Noop_t {
	template <class ...T> constexpr void operator()(const T&...) const {}
} Noop{};

constexpr struct MakeArray_t {
	template <class T, class... U> constexpr auto operator()(T&& t, U&&... u) const {
		return array<decay_t<T>, 1 + sizeof...(U)>{ forward<T>(t), forward<U>(u)... };
	}
} MakeArray{};

constexpr struct MakeUniquePtr_t {
	template <class T, class D> constexpr auto operator()(T* p, D d) const { return unique_ptr<T, D>{p, d}; }
} MakeUniquePtr{};

template <class Derived>
struct ConstantFn_t {
	template <class T, T t1, T ...t2> constexpr auto operator()(Constant<T, t1>, Constant<T, t2>...) const {
		constexpr auto result = Derived{}(t1, t2...);
		return Constant<decltype(result), result>{};
	}
};

constexpr struct And_t : ConstantFn_t<And_t> {
	using ConstantFn_t::operator();
	constexpr auto operator()() const { return True; }
	template <class T, class ...U> constexpr auto operator()(const T& t, const U&... u) const {
		bool b = static_cast<bool>(t);
		Noop((b = b && u)...);
		return b;
	}
} And{};

constexpr struct Or_t : ConstantFn_t<Or_t> {
	using ConstantFn_t::operator();
	constexpr auto operator()() const { return False; }
	template <class T, class ...U> constexpr auto operator()(const T& t, const U&... u) const {
		bool b = static_cast<bool>(t);
		Noop((b = b || u)...);
		return b;
	}
} Or{};

constexpr struct Multiply_t : ConstantFn_t<Multiply_t> {
	using ConstantFn_t::operator();
	template <class T, class ...U> constexpr T operator()(T t, const U& ...u) const {
		Noop((t *= u)...);
		return t;
	}
} Multiply{};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Functions

template <class T> string TypeName(Type<T>) {
	return MakeUniquePtr(abi::__cxa_demangle(typeid(T).name(), 0, 0, 0), free).get();
}

template <class T, T t> constexpr auto Value(Constant<T, t>) { return t; }
template <class T, T... t> constexpr auto ToArray(UnitList<Constant<T, t>...>) {
	return array<T, sizeof...(t)>{ t... };
}
template <class ...T> constexpr auto ToArray(UnitList<T...>) {
	return MakeArray(ToArray(T{})...);
}

template <class T, T t> constexpr auto operator!(Constant<T, t>) { return Constant<bool, !t>{}; }
template <class T, T t, class U, U u> constexpr auto operator<(Constant<T, t>, Constant<U, u>) {
	return Constant<bool, t < u>{};
}
template <class T, T t, class U, U u> constexpr auto operator>(Constant<T, t> a, Constant<U, u> b) { return b < a; }
template <class T, T t, class U, U u> constexpr auto operator<=(Constant<T, t> a, Constant<U, u> b) { return !(b < a); }
template <class T, T t, class U, U u> constexpr auto operator>=(Constant<T, t> a, Constant<U, u> b) { return !(a < b); }

template <class T> constexpr auto operator==(Unit<T>, Unit<T>) { return Constant<bool, true>{}; }
template <class T, class U> constexpr auto operator==(Unit<T>, Unit<U>) { return Constant<bool, false>{}; }
template <class T, class U> constexpr auto operator!=(Unit<T> t, Unit<U> u) { return !(t == u); }

template <class T> T DeclVal(Type<T>);
template <class T> constexpr auto Decay(Type<T>) { return Type<decay_t<T>>{}; }
template <class T> constexpr auto RemoveRValueReference(Type<T&&>) { return Type<T>{}; }
template <class T> constexpr auto RemoveRValueReference(Type<T>) { return Type<T>{}; }

template <class T, class Then, class Else> constexpr auto If(Constant<T, 0>, Then, Else v) { return v; }
template <class T, class Then, class Else> constexpr auto If(Constant<T, 1>, Then v, Else) { return v; }

template <class T, char c> constexpr auto Parse(Type<T>, UnitList<Constant<char, c>>) {
	return Constant<T, c - '0'>{};
}
template <char ...c> constexpr auto operator""_z() { return Parse(Type<size_t>{}, UnitList<Constant<char, c>...>{}); }

template <class T, T v, T w> constexpr auto operator+(Constant<T, v>, Constant<T, w>) {
	return Constant<T, v + w>{};
}

template <class ...T> constexpr auto Size(UnitList<T...>) { return Constant<size_t, sizeof...(T)>{}; }
template <class ...T> constexpr auto MakeList(Unit<T>...) { return UnitList<T...>{}; }
template <class ...T, class F> constexpr decltype(auto) Unpack(UnitList<T...>, F f) { return f(T{}...); }

template <class T> constexpr auto CommonType(Type<T> t) { return t; }
template <class T, class U> constexpr auto CommonType(Type<T>, Type<U>) {
	return Type<decltype(false ? declval<T>() : declval<U>())>{};
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
	return ApplyReference(t, ApplyCV(Type<remove_reference_t<T>>{}, u));
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
template <class T, class U, class ...W, class ...L>
constexpr auto CartesianProduct(UnitList<T, U, W...>, L...) {
	return Concatenate(CartesianProduct(UnitList<T>{}, L{}...),
					   CartesianProduct(UnitList<U, W...>{}, L{}...));
}

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
	constexpr Variant(nullptr_t) noexcept {}
	Variant(Variant&&) noexcept = default;
	template <class T, class U, class = enable_if_t<ValidType(Type<U>{})>>
	Variant(T&& t, Type<U> u) { data_ = { Tag(u), new U(forward<T>(t)) }; }
	template <class T> Variant(T&& t) : Variant(forward<T>(t), Decay(Type<T>{})) {}
	~Variant() {
		using FP = void (*)(void*);
		static const FP kFPs[] = { Noop, CastAndDelete<Types>... };
		kFPs[data_.tag](data_.buffer);
	}
	Variant& operator=(Variant _u) { swap(*this, _u); return *this; }
	friend void swap(Variant& _a, Variant& _b) { swap(_a.data_, _b.data_); }
	template <class T, class U>
	friend enable_if_t<Variant::ValidGet(Type<T>{}, Type<U&&>{}), T> UnsafeGetAs(Type<T>, U&& u) {
		return static_cast<T>(*static_cast<remove_reference_t<T>*>(u.data_.buffer));
	}
	template <class T, class U>
	friend enable_if_t<Variant::ValidGet(Type<T>{}, Type<U&&>{}), T> GetAs(Type<T>, U&& u) {
		if (u.data_.tag != Tag(Decay(Type<T>{})))
			throw bad_cast{};
		return static_cast<T>(*static_cast<remove_reference_t<T>*>(u.data_.buffer));
	}
	friend size_t BoundTypeIndex(const Variant& v) { return v.data_.tag; }
};

template <class ...T> constexpr auto BoundTypes(Type<Variant<T...>>) { return UnitList<Type<T>...>{}; }

template <class T> constexpr size_t BoundTypeIndex(const T&) { return 0; }
template <class T> constexpr auto BoundTypes(Type<T>) { return UnitList<Type<T>>{}; }

template <class T, class U> enable_if_t<Type<T>{} == Type<U&&>{}, T> UnsafeGetAs(Type<T>, U&& u) {
	return static_cast<T>(u);
}

template <class T, class U> enable_if_t<Type<T>{} == Type<U&&>{}, T> GetAs(Type<T>, U&& u) {
	return static_cast<T>(u);
}

template <class A> decltype(auto) Subscript(A&& a, size_t s) { return forward<A>(a)[s]; }
template <class A, class ...S> decltype(auto) Subscript(A&& a, size_t s1, size_t s2, S ...s3) {
	return Subscript(forward<A>(a)[s1], s2, s3...);
}

template <class F, class ...V> class MultiMethod {
	constexpr static auto all_ = CartesianProduct(BoundTypes(Decay(Type<V>{}))...);
	using R = decltype(DeclVal(Unpack(all_, [](auto ...al) {
		return RemoveRValueReference(CommonType(Unpack(al, [](auto ...a) {
			return ResultType(Type<F>{}, MakeList(ApplyCVReference(Type<V&&>{}, a)...));
		})...));
	})));
	template <class ...A> static R Imp(V&& ...v) {
		return F{}(UnsafeGetAs(ApplyCVReference(Type<V&&>{}, A{}), forward<V>(v))...);
	}
	[[noreturn]] static R NullImp(V&&...) { throw invalid_argument("Null Variant passed to function."); }
	template <class AL> constexpr static auto GetImpRecur(AL al) {
		return decltype(Unpack(al, [](auto... a) {
			return If(And((a != Type<void>{})...),
					  [](auto... b) { return Constant<R (*)(V&&...), Imp<decltype(b)...>>{}; },
					  [](auto...) { return Constant<R (*)(V&&...), NullImp>{}; })(a...);
		})){};
	}
	template <class AL, class BL1, class ...BL2> constexpr static auto GetImpRecur(AL, BL1, BL2...) {
		return decltype(Unpack(BL1{}, [=](auto... a) {
			return MakeList(GetImpRecur(Append(Type<void>{}, AL{}), BL2{}...),
							GetImpRecur(Append(decltype(a){}, AL{}), BL2{}...)...);
		})){};
	}
public:
	static auto GetImp(const V& ...v) {
		constexpr static auto imps = ToArray(GetImpRecur(UnitList<>{}, BoundTypes(Decay(Type<V>{}))...));
		return Subscript(imps, BoundTypeIndex(v)...);
	}
};

#if 1
#define DEFINE_FUNCTION(fn)\
	namespace Fn {\
		struct PPCAT(fn,_t) {\
			template <class ...A> constexpr decltype(auto) operator()(A&& ...a) const { return fn(forward<A>(a)...); }\
		};\
	}
#define DEFINE_MULTIMETHOD(fn)\
	DEFINE_FUNCTION(fn)\
	namespace VariantNS {\
		template <class ...V> constexpr decltype(auto) fn(V&& ...v) {\
			return MultiMethod<Fn::PPCAT(fn,_t), V...>::GetImp(v...)(forward<V>(v)...);\
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
