#include "ApplyCVReference.h"
#include "Array.h"
#include "CommonType.h"
#include "Constant.h"
#include "ConstantFunction.h"
#include "ConstantList.h"
#include "Decay.h"
#include "DeclVal.h"
#include "FindArg.h"
#include "GetNthArg.h"
#include "MakeArray.h"
#include "MakeIndexList.h"
#include "MakeList.h"
#include "Multiply.h"
#include "PPCAT.h"
#include "RemoveRValueReference.h"
#include "ResultType.h"
#include "Size.h"
#include "SizeConstant.h"
#include "Sum.h"
#include "Type.h"
#include "With.h"
#include "_z.h"
#include <algorithm>
#include <array>
#include <assert.h>
#include <iostream>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <utility>
#include <algorithm>
class Unique_t {
	template <class... X>
	struct Data {
		template <class T> static auto findArg(T) {
			return FindArg(T{}, X{}...);
		}
		size_t indices[sizeof...(X)] = {
			decltype(findArg(X{}))::Value()...
		};
		size_t count = 0;
		constexpr Data() {
			for (size_t i = 0; i < sizeof...(X); ++i) {
				if (indices[i] == i)
					indices[count++] = i;
			}
		}
	};
public:
	template <class... X> auto operator()(Constant<X>...) const {
		constexpr Data<X...> data{};
		return MakeIndexList(SizeConstant<data.count>{}) | [&](auto... i) {
			return MakeList(GetNthArg(SizeConstant<data.indices[i]>{}, X{}...)...);
		};
	}
};
constexpr static Unique_t Unique{};
/*
class Concatenate_t {
	template <class... S> constexpr static auto MakeIndexArrayArray() {
		Array<Array<size_t, 2>, Sum(S{}...)> array{};
		size_t sizes[] = { S{}... };
		for (size_t n = 0, i = 0; i < sizeof...(S); ++i)
			for (size_t j = 0; j < sizes[i]; ++j, ++n)
				array[n] = {{ i, j }};
		return array;
	}
	template <class... S> static auto MakeIndexListList(S...) {
		constexpr static auto indexArrayArray = MakeIndexArrayArray<S...>();
		return MakeIndexList(Sum(S{}...)) | [](auto... n) {
			return MakeList(With(n) | [](auto n) {
				constexpr auto indexArray = indexArrayArray[n];
				constexpr auto i = SizeConstant<indexArray[0]>{};
				constexpr auto j = SizeConstant<indexArray[1]>{};
				return MakeList(i, j);
			}...);
		};
	}
public:
	template <class... L> auto operator()(L... l) const {
		auto ll = MakeList(l...);
		return MakeIndexListList(Size(l)...) | [&](auto... il) {
			return MakeList(il | [&](auto i, auto j) {
				return ll[i][j];
			}...);
		};
	}
};
constexpr static Concatenate_t Concatenate_2{};
*/
class CartesianProduct_t {
	template <size_t... s> constexpr static auto MakeIndexArray(size_t i) {
		constexpr size_t sizes[] = { s... };
		constexpr size_t n = sizeof...(s);
		Array<size_t, n> result{};
		for (size_t j = n; j--;) {
			result[j] = i % sizes[j];
			i /= sizes[j];
		}
		return result;
	}
public:
	template <class... L> auto operator()(L... l) const {
		auto makePermutation = [&l...](auto... i) {
			return MakeList(l[i]...);
		};
		constexpr static auto elementIndexList = MakeIndexList(SizeConstant<sizeof...(L)>{});
		auto makeElement = [&](auto i) {
			constexpr static auto indexArray = MakeIndexArray<decltype(Size(l))::Value()...>(i);
			return elementIndexList | [&](auto... j) {
				return makePermutation(SizeConstant<indexArray[j]>{}...);
			};
		};
		return MakeIndexList(Multiply(Size(l)...)) | [&](auto... i) {
			return MakeList(makeElement(i)...);
		};
	}
};
constexpr static CartesianProduct_t CartesianProduct{};
namespace InvalidNS {
struct Invalid {
	template <class T> operator T() const { throw ""; }
};
}
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
	constexpr static auto typeList_ = MakeList(type<Types>...);
	constexpr static auto typeCount_ = Size(typeList_);
	constexpr static auto Tag(Type<InvalidNS::Invalid>) { return 0_z; }
	template <class T> constexpr static auto Tag(Type<T> t) { return FindArg(t, type<Types>...) + 1_z; }
	template <class T> constexpr static auto ValidType(Type<T> t) { return Variant::Tag(t) <= typeCount_; }
	template <class To, class From> static constexpr auto ValidGet(Type<To> to, Type<From> from) {
		constexpr auto toval = Decay(Type<To>{});
		constexpr auto fromval = Decay(Type<From>{});
		static_assert(Type<Variant>{} == fromval, "Calling GetAs with wrong Variant type!");
		return Variant::ValidType(toval) && ApplyCVReference(from, toval) == to;
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
template <class ...T> constexpr auto BoundTypes(Type<Variant<T...>>) { return ConstantList<Type<InvalidNS::Invalid>, Type<T>...>{}; }
template <class T> constexpr size_t BoundTypeIndex(const T&) { return 0; }
template <class T> constexpr auto BoundTypes(Type<T>) { return ConstantList<Type<T>>{}; }
template <class T, class U> std::enable_if_t<Type<T>{} == Type<U&&>{}, T> UnsafeGetAs(Type<T>, U&& u) {
	return static_cast<T>(u);
}
template <class T, class U> std::enable_if_t<Type<T>{} == Type<U&&>{}, T> GetAs(Type<T>, U&& u) {
	return static_cast<T>(u);
}
template <class R, class ...V> struct MultiMethodNullImp {
	static R value(V...) { throw std::invalid_argument("Null Variant passed to function."); }
};
template <class F, class R, class ...V> struct MultiMethodImp {
	template <class ...A> struct WithArgs {
		static R value(V ...v) {
			return F{}(UnsafeGetAs(A{}, static_cast<V>(v))...);
		}
	};
};
template <class F, class ...V> struct MultiMethodImp<F, void, V...> {
	template <class ...A> struct WithArgs {
		static void value(V ...v) {
			F{}(UnsafeGetAs(A{}, static_cast<V>(v))...);
		}
	};
};
struct FlattenIndices_t {
	template <size_t count, class... S>
	constexpr static size_t invoke(const Array<size_t, count> sizes, S... index) {
		static_assert(count == sizeof...(S), "");
		size_t indices[] = { index... };
		size_t r = 0;
		size_t i = count;
		size_t m = 1;
		while (i--) {
			r += m * indices[i];
			m *= sizes[i];
		}
		return r;
	}
};
constexpr static ConstantFunction<FlattenIndices_t> FlattenIndices{};
struct InvokeMultiMethod_t {
	template <class F, class... V> constexpr auto operator()(F, V&&... v) const {
		auto getBoundTypeList = [](auto v) {
			return BoundTypes(Decay(v)) | [&](auto... b) {
				return MakeList(ApplyCVReference(v, b)...);
			};
		};
		auto boundTypeListList = MakeList(getBoundTypeList(Type<V&&>{})...);
		auto argTypeListList = boundTypeListList | [](auto... bl) {
			return CartesianProduct(bl...);
		};
		auto getResultType = [](auto al) {
			return al | [](auto... a) {
				return ResultType(Type<F>{}, a...);
			};
		};
		using R = decltype(DeclVal(RemoveRValueReference(argTypeListList | [&](auto... al) {
			return Unique(getResultType(al)...) | [](auto... r) {
				return CommonType(r...);
			};
		})));
		auto getImp = [&](auto al) {
			return al | [](auto... a) {
				return VCONSTANT(&MultiMethodImp<F, R, V&&...>::template WithArgs<decltype(a)...>::value){};
			};
		};
		constexpr auto imps = decltype(argTypeListList | [&](auto... al) {
			return MakeArray(getImp(al)...);
		})::Value();
		constexpr auto sizes = decltype(boundTypeListList | [&](auto... bl) {
			return MakeArray(Size(bl)...);
		})::Value();
		size_t index = FlattenIndices(sizes, BoundTypeIndex(v)...);
		return imps[index](static_cast<V&&>(v)...);
	}
};
constexpr static InvokeMultiMethod_t InvokeMultiMethod{};
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
			return InvokeMultiMethod(Hel::PPCAT(fn,_ft){}, static_cast<V&&>(v)...);\
		}\
	}\
	namespace InvalidNS {\
		template <class ...V> constexpr Invalid fn(V&& ...v) {\
			throw std::invalid_argument("Null Variant passed to function.");\
			return {};\
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
inline void Print(S1) {
	puts(__PRETTY_FUNCTION__);
}
inline void Print(S2) {
	puts(__PRETTY_FUNCTION__);
}
inline void Print(S3) {
	puts(__PRETTY_FUNCTION__);
}
inline void Print(S4) {
	puts(__PRETTY_FUNCTION__);
}
inline void Print(S5) {
	puts(__PRETTY_FUNCTION__);
}
inline void Print(S6) {
	puts(__PRETTY_FUNCTION__);
}
inline void Print(S7) {
	puts(__PRETTY_FUNCTION__);
}
inline void Print(S8) {
	puts(__PRETTY_FUNCTION__);
}
inline void Print(S9) {
	puts(__PRETTY_FUNCTION__);
}
inline void Print(S10) {
	puts(__PRETTY_FUNCTION__);
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
	//cout << Name(Type<decltype(testb)>{}) << endl;
#if 1
	{
		Shape3 s;
		s = Circle{};
		Print(s);
	}
#endif
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
#if 0
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
		cout << "----------" << endl;
		s7 = move(s3);
		cout << "----------" << endl;
	}
	cout << "----------" << endl;
#endif
}
