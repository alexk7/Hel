#include "And.h"
#include "Array.h"
#include "CharConstant.h"
#include "Constant.h"
#include "ConstantFunction.h"
#include "False.h"
#include "MakeArray.h"
#include "MakeUniquePtr.h"
#include "PPCAT.h"
#include "SizeConstant.h"
#include "Sum.h"
#include "True.h"
#include "Type.h"
#include "TypeName.h"
#include "Unit.h"
#include "UnitList.h"
#include "With.h"
#include <algorithm>
#include <array>
#include <assert.h>
#include <iostream>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <utility>

template <class T> T DeclVal(Type<T>);
template <class T> constexpr auto Decay(Type<T>) { return Type<std::decay_t<T>>{}; }
template <class T> constexpr auto RemoveRValueReference(Type<T&&>) { return Type<T>{}; }
template <class T> constexpr auto RemoveRValueReference(Type<T>) { return Type<T>{}; }

struct If_t {
	template <class T, class U> constexpr static decltype(auto) invoke(BoolConstant<true>, T&& t, U&&) {
		return static_cast<T&&>(t);
	}
	template <class T, class U> constexpr static decltype(auto) invoke(BoolConstant<false>, T&&, U&& u) {
		return static_cast<U&&>(u);
	}
	template <class C, class T, class U> constexpr static auto invoke(Constant<C>, T&& t, U&& u) {
		return invoke(BoolConstant<static_cast<bool>(C::value)>{}, static_cast<T&&>(t), static_cast<U&&>(u));
	}
};
constexpr static ConstantFunction<If_t> If{};

template <class T, char c> constexpr auto Parse(Type<T>, UnitList<CharConstant<c>>) {
	return TypedConstant<T, c - '0'>{};
}
template <char ...c> constexpr auto operator""_z() { return Parse(Type<size_t>{}, UnitList<CharConstant<c>...>{}); }

template <class T, class U> constexpr auto operator+(Constant<T> a, Constant<U> b) {
	return Sum(a, b);
}

template <class ...T> constexpr auto ArgCount(const T&...) { return SizeConstant<sizeof...(T)>{}; }
constexpr auto NoArgs() { return True; }
constexpr auto NoArgs(...) { return False; }
constexpr auto Empty(UnitList<>) { return True; }
template <class ...T> constexpr auto Empty(UnitList<T...>) { return False; }
template <class ...T> constexpr auto Size(UnitList<T...>) { return SizeConstant<sizeof...(T)>{}; }
template <class ...T> constexpr auto MakeList(Unit<T>...) { return UnitList<T...>{}; }
template <class F, class... T> constexpr auto operator|(UnitList<T...>, F&& f) { return static_cast<F&&>(f)(T{}...); }

/*
constexpr static struct Identity_t {
	template <class T> constexpr T operator()(T&& t) const { return static_cast<T&&>(t); }
} Identity{};
*/

#define IF(Cond, Then, Else) If(Cond, [&](auto) { return Then; }, [&](auto) { return Else; })(0)

template <class ...T, class ...U> constexpr auto Concatenate(UnitList<T...>, UnitList<U...>) {
	return UnitList<T..., U...>{};
}

template <class F> class Recursive {
	F f_;
public:
	Recursive(const F& f) : f_(f) {}
	Recursive(F&& f) : f_(static_cast<F&&>(f)) {}
	template <class ...A> constexpr auto operator()(A&& ...a) const { return f_(*this, static_cast<A&&>(a)...); }
};

template <class F> constexpr auto MakeRecursive(F&& f) { return Recursive<std::decay_t<F>>{static_cast<F&&>(f)}; }

struct Quotient_t {
	template <class T, class U> constexpr static auto invoke(const T& t, const U& u) { return t / u; }
};
constexpr static ConstantFunction<Quotient_t> Quotient{};
template <class A, class B> constexpr auto operator/(Constant<A> a, Constant<B> b) { return Quotient(a, b); }
struct Difference_t {
	template <class T, class U> constexpr static auto invoke(const T& t, const U& u) { return t - u; }
};
constexpr static ConstantFunction<Difference_t> Difference{};
template <class A, class B> constexpr auto operator-(Constant<A> a, Constant<B> b) { return Difference(a, b); }

template <class T> constexpr auto CommonType(Type<T> t) { return t; }
template <class T, class U, class ...V> constexpr auto CommonType(Type<T> t, Type<U> u, Type<V> ...v) {
	return CommonType(Type<decltype(false ? DeclVal(t) : DeclVal(u))>{}, v...);
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

/*
struct Iota_t {
	template <size_t n> struct Result {
		constexpr static auto invoke() {
			Array<size_t, n> result = {{ 0 }};
			for (size_t i = 0; i < n; ++i)
				result.value[i] = i;
			return result;
		}
		constexpr static auto value = invoke();
	};
public:
	template <class N> auto operator()(N) const {
		return Constant<Result<N::value>>{};
	}
} Iota;
*/

//constexpr auto test = IotaArray(Type<Array<size_t,

/*
static auto IotaList = [](auto n) {
	return IF(n == 0_z, MakeList(), IF(n == 1_z, MakeList(0_z), With(n) | [](auto n) {
		return MakeIndexList(n / 2_z) | [&](auto... a) {
			return MakeIndexList(n - n / 2_z) | [&](auto... b) {
				auto aCount = ArgCount(a...);
				return MakeList(a..., (aCount + b)...);
			};
		};
	}));
};

static auto ArrayToList = [](auto a) {


	constexpr auto indexArray = MakeIndexArray(n);
	return MakeList(
}


// Scheme for variable names:
// i: Input
// o: Output
// l: List
// r: Recursive call result

static auto Prepend_2 = [](auto xl, auto y) {
	return xl | [&](auto... x) {
		return MakeList(y, x...);
	};
};
static auto Map = [](auto xl, auto f) {
	return xl | [&](auto... x) {
		return MakeList(f(x)...);
	}
}

static auto ConcatenateN = [](auto xll, auto n) {
	return IF(n == 0_z, MakeList(), IF(n == 1_z, xll | [](auto xl, auto...) {
		return xl;
	}, ConcatenateN(ConcatenateN(xll, n / 2_z), Concatenate(GetNthTailFn(n / 2_z), n - n / 2_z)
};
static auto Concatenate_2 = MakeRecursive([](auto concatenate, auto xll) {
	MakeIndexList(Size(xll))
	return xll | [&](auto... xl) {
		return IF(NoArgs(xl...), MakeList(), With(xl...) | [&](auto xl1, auto... xl2) {
			return IF(NoArgs(xl2...), xl1,
		});
	};
});

static auto Concatenate_2 = MakeRecursive([](auto concatenate, auto xll) {
	return xll | [&](auto... xl) {
		return IF(NoArgs(xl...), MakeList(), With(xl...) | [&](auto xl1, auto... xl2) {
			return IF(NoArgs(xl2...), xl1,
		});
	};
});
*/
/*
static auto CartesianProduct_2 = MakeRecursive([](auto cartesianProduct, auto il1, auto... il2) {
	auto iter = [&](auto i1) {
		return cartesianProduct(il2...) | [&](auto... rl) {
			return MakeList(rl | [&](auto... r) {
				return MakeList(i1, r...);
			}...);
		};
	};
	return IF(NoArgs(il2...), MakeList(il1), With((il1 | iter)...) | [&](auto... rll) {
		return rll | [&](auto... rl) {
			return MakeList(rl...);
		};
	});
});
*/

/*
static auto Identity = [](auto x) {
	return x;
};
static auto Tail = [](auto xl) {
	return xl | [](auto, auto... x) {
		return MakeList(x...);
	};
};
*/
/*
static auto GetNthElement = [](auto n, auto xl) {
	auto getNthTailFn = MakeRecursive([](auto getNthTailFn, auto n) {
		return IF(n == 0_z, Identity, IF(n == 1_z, Tail, [=](auto xl) {
			return getNthTailFn(n - n / 2_z)(getNthTailFn(n / 2_z)(xl));
		}));
	});
	return xl | getNthTailFn(n) | [](auto x, auto...) {
		return x;
	};
};
*/

static auto MakeIndexList = MakeRecursive([](auto MakeIndexList, auto n) {
	return IF(n > 1_z, With(n) | [&](auto n) {
		auto h = n / 2_z;
		auto al = MakeIndexList(h);
		auto bl = MakeIndexList(n - h);
		auto aSize = Size(al);
		return al | [&](auto... a) {
			return bl | [&](auto... b) {
				return MakeList(a..., (aSize + b)...);
			};
		};
	}, IF(n == 1_z, MakeList(0_z), MakeList()));
});

template <class... KV> class UnitMap {
	struct KVHack : KV... {};
	template <class K, class V> static V VHack(UnitList<K, V>); //undefined
public:
	template <class K> auto operator[](K) const { return decltype(VHack<K>(KVHack{})){}; }
};
template <class... K, class... V> auto MakeMap(UnitList<K, V>...) { return UnitMap<UnitList<K, V>...>{}; }

static auto MakeUnitArray = [](auto... t) {
	return MakeIndexList(ArgCount(t...)) | [t...](auto... i) {
		return MakeMap(MakeList(i, t)...);
	};
};

template <class To, class From> auto StaticCast(Type<To>, From from) { return static_cast<To>(from); }

template <class XL, class N> auto GetNthElement(XL xl, N n) {
	return xl | [&](auto... x) {
		auto types = MakeUnitArray(Type<decltype(x)*>{}...);
		void* values[] = { &x... };
		return *StaticCast(types[n], values[n]);
	};
};

//template <class T, size_t s> constexpr auto Size(Array<T, s>) { return SizeConstant<s>{}; }

static class Concatenate_t {
	struct IJ {
		size_t i;
		size_t j;
	};
	template <class... S> struct IJArray {
		constexpr static auto Value() {
			Array<IJ, Sum(S{}...)> array{};
			size_t sizes[] = { S{}... };
			for (size_t n = 0, i = 0; i < sizeof...(S); ++i)
				for (size_t j = 0; j < sizes[i]; ++j, ++n)
					array.value[n] = {i, j};
			return array;
		}
		constexpr static auto value = Value();
	};
	template <class... S> static auto MakeIJList(S...) {
		constexpr static auto array = IJArray<S...>::value;
		return MakeIndexList(Sum(S{}...)) | [](auto... n) {
			return MakeList(MakeList(SizeConstant<array.value[n].i>{},
									 SizeConstant<array.value[n].j>{})...);
		};
	}
public:
	template <class... L> auto operator()(L... l) const {
		auto ll = MakeList(l...);
		return MakeIJList(Size(l)...) | [&](auto... ij) {
			return MakeList(ij | [&](auto i, auto j) {
				return GetNthElement(GetNthElement(ll, i), j);
			}...);
		};
	}
} Concatenate_2{};

//*
static auto test = Concatenate_2(MakeList(Type<int>{}, Type<void>{}),
								 MakeList(Type<char>{}, Type<double>{}, Type<long long>{}),
								 MakeList(Type<unsigned>{}));
//*/

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

template <class R, class ...V> struct MultiMethodNullImp {
	static R value(V...) { throw std::invalid_argument("Null Variant passed to function."); }
};

template <class F, class R, class ...V> struct MultiMethodImp {
	template <class ...A> struct WithArgs {
		static R value(V ...v) {
			return F{}(UnsafeGetAs(ApplyCVReference(Type<V>{}, A{}), static_cast<V>(v))...);
		}
	};
};

template <class F, class... V> static auto InvokeMultiMethod(V&&... v) {
	auto boundTypeListList = MakeList(BoundTypes(Decay(Type<V>{}))...);
	//auto all = CartesianProduct_2(boundTypeListList);
	auto getCommonResultType = MakeRecursive([&](auto getCommonResultType, auto bll, auto al) {
		return IF(Empty(bll), al | [&](auto... a) {
			return ResultType(Type<F>{}, MakeList(ApplyCVReference(Type<V&&>{}, a)...));
		}, bll | [&](auto bl0, auto... bl1) {
			return bl0 | [&](auto... b) {
				return al | [&](auto... a) {
					return CommonType(getCommonResultType(MakeList(bl1...), MakeList(a..., b))...);
				};
			};
		});
	});
	using R = decltype(DeclVal(RemoveRValueReference(getCommonResultType(boundTypeListList, MakeList()))));
	using NullImp = MultiMethodNullImp<R, V&&...>;
	auto getNullImps = MakeRecursive([&](auto getNullImps, auto bll) {
		return IF(Empty(bll), GetConstant(NullImp{}), bll | [&](auto bl0, auto... bl1) {
			auto nullImps = getNullImps(MakeList(bl1...));
			return bl0 | [&](auto... b) {
				return MakeArray(nullImps, ((void)b, nullImps)...);
			};
		});
	});
	auto getImps = MakeRecursive([&](auto getImps, auto bll, auto al) {
		return IF(Size(bll) == 0_z, al | [&](auto... a) {
			return GetConstant(typename MultiMethodImp<F, R, V&&...>::template WithArgs<decltype(a)...>{});
		}, bll | [&](auto bl0, auto... bl1) {
			auto tail = MakeList(bl1...);
			return bl0 | [&](auto... b) {
				return al | [&](auto... a) {
					return MakeArray(getNullImps(tail), getImps(tail, MakeList(a..., b))...);
				};
			};
		});
	});
	constexpr static auto imps = decltype(getImps(boundTypeListList, MakeList()))::value;
	//*
	auto findImp = MakeRecursive([&](const auto& callImp, const auto& imps, size_t i1, auto... i2) {
		const auto& found = imps.value[i1];
		return IF(ArgCount(i2...) == 0_z, found, With(found) | [&](auto& found) {
			return callImp(found, i2...);
		});
	});
	return findImp(imps, BoundTypeIndex(v)...)(static_cast<V&&>(v)...);
	//*/
	/*
	auto findImpIndex = MakeRecursive([&](const auto& findImpIndex, const auto& v1, const auto&... v2) {
		BoundTypeIndex
		const auto& found = imps.value[i1];
		return IF(ArgCount(i2...) == 0_z, found, With(found) | [&](auto& found) {
			return callImp(found, i2...);
		});
	});
	return imps[findImpIndex(v...)](static_cast<V&&>(v)...);
	//*/
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

#if 1
DEFINE_MULTIMETHOD(Print)
DEFINE_MULTIMETHOD(Index)
DEFINE_MULTIMETHOD(Intersect)
DEFINE_MULTIMETHOD(Copy)
DEFINE_MULTIMETHOD(Test3)
#endif

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

using Shape2 = Variant<Circle, Rectangle>;
using Shape3 = Variant<Circle, Rectangle, S1, S2, S3, S4, S5, S6, S7, S8, S9, S10, Triangle>;
using Shape3b = Variant<Circle, Rectangle, Triangle>;

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

inline bool Intersect(const Triangle&, const Triangle&) {
	puts(__PRETTY_FUNCTION__);
	return false;
}

template <class T>
inline bool Intersect(const Triangle&, const T&) {
	puts(__PRETTY_FUNCTION__);
	return false;
}

template <class T>
inline bool Intersect(const T&, const Triangle&) {
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

inline void Test3(const Shape3&, const Shape3&, const Shape3&) {
	puts(__PRETTY_FUNCTION__);
}

inline void Test3(Circle, Rectangle, Triangle) {
	puts(__PRETTY_FUNCTION__);
}

int main()
{
	cout << TypeName(Type<decltype(test)>{}) << endl;

#if 1
	{
		Shape2 v, v2, v3;
		v = Circle{};
		v2 = Rectangle{};
		cout << Intersect(v, v) << endl;
		cout << Intersect(v, v2) << endl;
		cout << Intersect(v2, v2) << endl;
		cout << Intersect(v2, v) << endl;
		cout << Intersect(move(v2), v) << endl;
	}
#endif

#if 1
	{
		Shape3 v, v2, v3;
		v = Circle{};
		v2 = Rectangle{};
		v3 = Triangle{};
		Test3(v, v, v);
		Test3(v, v2, v3);
	}
#endif

#if 1
	{
		Shape3b v, v2, v3;
		v = Circle{};
		v2 = Rectangle{};
		v3 = Triangle{};
		Test3(v, v, v);
		Test3(v, v2, v3);
	}
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
			Intersect(s9, s2);
			Print(s9);
			cout << Index(s9) << endl;
			cout << Index(move(s9)) << endl;
		} catch (const exception& e) {
			puts(e.what());
		}

		//cout << "----------" << endl;
		//s7 = move(s3);

		//cout << "----------" << endl;c
	}

	cout << "----------" << endl;
#endif
}
