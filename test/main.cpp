#include "_z.h"
#include "Array.h"
#include "CharConstant.h"
#include "Constant.h"
#include "False.h"
#include "MakeArray.h"
#include "MakeIndexList.h"
#include "MakeUniquePtr.h"
#include "Multiply.h"
#include "PPCAT.h"
#include "SizeConstant.h"
#include "Sum.h"
#include "True.h"
#include "Type.h"
#include "TypeName.h"
#include "ConstantList.h"
#include "With.h"
#include <algorithm>
#include <array>
#include <assert.h>
#include <iostream>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <utility>
#include <algorithm>

template <class T> T DeclVal(Type<T>);
template <class T> constexpr auto Decay(Type<T>) { return Type<std::decay_t<T>>{}; }
template <class T> constexpr auto RemoveRValueReference(Type<T&&>) { return Type<T>{}; }
template <class T> constexpr auto RemoveRValueReference(Type<T>) { return Type<T>{}; }

struct If_t {
	template <class T, class U> constexpr T&& operator()(BoolConstant<true>, T&& t, U&&) const {
		return static_cast<T&&>(t);
	}
	template <class T, class U> constexpr U&& operator()(BoolConstant<false>, T&&, U&& u) const {
		return static_cast<U&&>(u);
	}
	template <class C, class T, class U> constexpr decltype(auto) operator()(Constant<C>, T&& t, U&& u) const {
		return (*this)(BoolConstant<static_cast<bool>(C::Value())>{}, static_cast<T&&>(t), static_cast<U&&>(u));
	}
};
constexpr static If_t If{};

template <class... T> constexpr auto ArgCount(const T&...) { return SizeConstant<sizeof...(T)>{}; }
constexpr auto NoArgs() { return True; }
constexpr auto NoArgs(...) { return False; }
constexpr auto Empty(ConstantList<>) { return True; }
template <class... T> constexpr auto Empty(ConstantList<T...>) { return False; }
template <class... T> constexpr auto Size(ConstantList<T...>) { return SizeConstant<sizeof...(T)>{}; }
template <class... T> constexpr auto MakeList(Constant<T>...) { return ConstantList<T...>{}; }
template <class F, class... T> constexpr auto operator|(ConstantList<T...>, F&& f) { return static_cast<F&&>(f)(T{}...); }

/*
constexpr static struct Identity_t {
	template <class T> constexpr T operator()(T&& t) const { return static_cast<T&&>(t); }
} Identity{};
*/

#define IF(Cond, Then, Else) If(Cond, [&](auto) { return Then; }, [&](auto) { return Else; })(0)

/*
template <class ...T, class ...U> constexpr auto Concatenate(ConstantList<T...>, ConstantList<U...>) {
	return ConstantList<T..., U...>{};
}
*/

template <class F> class Recursive {
	F f_;
public:
	Recursive(const F& f) : f_(f) {}
	Recursive(F&& f) : f_(static_cast<F&&>(f)) {}
	template <class ...A> constexpr auto operator()(A&& ...a) const { return f_(*this, static_cast<A&&>(a)...); }
};

template <class F> constexpr auto MakeRecursive(F&& f) { return Recursive<std::decay_t<F>>{static_cast<F&&>(f)}; }

//*
template <class T> constexpr auto CommonType(Type<T> t) { return t; }
template <class T, class U, class ...V> constexpr auto CommonType(Type<T> t, Type<U> u, Type<V> ...v) {
	return CommonType(Type<decltype(false ? DeclVal(t) : DeclVal(u))>{}, v...);
}
//*/

/*
template <class T, class... U, class = std::enable_if_t<Equal(Type<T>{}, Type<U>{}...)>>
constexpr auto CommonType(Type<T> t, Type<U>...) { return t; }
template <class T, class U> constexpr auto CommonType(Type<T> t, Type<U> u) {
	return Type<decltype(false ? DeclVal(t) : DeclVal(u))>{};
}
template <class T, class... U, class = std::enable_if_t<!Equal(Type<T>{}, Type<U>{}...)>>
constexpr auto CommonType(Type<T> t, Type<U>... u) {
	return CommonType(Type<decltype(false ? DeclVal(t) : DeclVal(u))>{}, v...);
}
//*/

template <class F, class ...A> constexpr auto ResultType(Type<F> f, Type<A>... a) {
	return Type<decltype(DeclVal(f)(DeclVal(a)...))>{};
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

struct FindArg_t {
	template <class T, class... X> constexpr static auto invoke(T t, X... x) {
		size_t i = 0;
		bool done = false;
 		(void)(size_t[]){ (done ? 0 : t == x ? done = true : ++i)... };
		return i;
	}
};
constexpr static ConstantFunction<FindArg_t> FindArg{};

/*
static auto MakeIndexList = MakeRecursive([](auto MakeIndexList, auto n) {
	return IF(n > 1_z, With(n) | [&](auto n) {
		auto h = n / 2_z;
		auto al = MakeIndexList(h);
		auto bl = MakeIndexList(n - h);
		return al | [&](auto... a) {
			return bl | [&](auto... b) {
				return MakeList(a..., (h + b)...);
			};
		};
	}, IF(n == 1_z, MakeList(0_z), MakeList()));
});
*/

template <class... KV> class ConstantMap {
	struct KVHack : KV... {};
	template <class K, class V> static V VHack(ConstantList<K, V>); //undefined
public:
	template <class K> auto operator[](K) const { return decltype(VHack<K>(KVHack{})){}; }
};
template <class... K, class... V> auto MakeMap(ConstantList<K, V>...) { return ConstantMap<ConstantList<K, V>...>{}; }

static auto MakeConstantArray = [](auto... c) {
	return MakeIndexList(ArgCount(c...)) | [c...](auto... i) {
		return MakeMap(MakeList(i, c)...);
	};
};

template <class To, class From> auto StaticCast(Type<To>, From from) { return static_cast<To>(from); }

#define UNIT_ASSERT(x) static_assert(decltype(x){}, #x)

template <class N, class... X> auto GetNthArg(N n, X... x) {
	UNIT_ASSERT(n < ArgCount(x...));
	auto types = MakeConstantArray(Type<X*>{}...);
	void* values[] = { &x... };
	return *StaticCast(types[n], values[n]);
};

template <class N, class XL> auto GetNthElement(N n, XL xl) {
	return xl | [&](auto... x) {
		return GetNthArg(n, x...);
	};
};

//*
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
//*/

//template <class T, size_t s> constexpr auto Size(Array<T, s>) { return SizeConstant<s>{}; }

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


/*
class CartesianProduct_t {
	constexpr static void FillIndexArray(size_t* result, const size_t* sizes, size_t P, size_t N) {
		for (size_t i = 0; i < P; ++i) {
			size_t now = i;
			for (size_t j = N; j--;) {
				result[i * N + j] = now % sizes[j];
				now /= sizes[j];
			}
		}
	}
	template <size_t... s> constexpr static auto MakeIndexArray() {
		constexpr size_t P = Multiply(s...);
		constexpr size_t N = sizeof...(s);
		constexpr size_t sizes[] = { s... };
		Array<size_t, P * N> result{};
		FillIndexArray(result.values, sizes, P, N);
		return result;
	}
	template <class... S> static auto MakeIndexListList(S...) {
		constexpr static auto N = ArgCount(S::Value()...);
		constexpr static auto indexArray = MakeIndexArray<S::Value()...>();
		return MakeIndexList(Multiply(S{}...)) | [](auto... i) {
			return MakeList(With(i) | [](auto i) {
				return MakeIndexList(N) | [&](auto... j) {
					return MakeList(SizeConstant<indexArray[decltype(i * N + j)::Value()]>{}...);
				};
			}...);
		};
	}
public:
	template <class... L> auto operator()(L... l) const {
		return MakeIndexListList(Size(l)...) | [&l...](auto... il) {
			return MakeList(il | [&l...](auto... i) {
				return MakeList(l[i]...);
			}...);
		};
	}
};
constexpr static CartesianProduct_t CartesianProduct{};
//*/

//*
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
		auto makeElement = [&makePermutation](auto i) {
			constexpr static auto indexArray = MakeIndexArray<decltype(Size(l))::Value()...>(i);
			return elementIndexList | [&makePermutation](auto... j) {
				return makePermutation(SizeConstant<indexArray[j]>{}...);
			};
		};
		return MakeIndexList(Multiply(Size(l)...)) | [&makeElement](auto... i) {
			return MakeList(makeElement(i)...);
		};
	}
};
constexpr static CartesianProduct_t CartesianProduct{};
//*/

/*
static auto testb = MakeIndexList(SizeConstant<257>{}) | [](auto j, auto... i) {
	return 0;
};
//*/
/*
static auto test = CartesianProduct(
	MakeList(type<long long>, type<unsigned>, type<void*>, type<char*>),
	MakeList(type<long long>, type<unsigned>, type<void*>, type<char*>),
	MakeList(type<long long>, type<unsigned>, type<void*>, type<char*>, type<int>, type<void>, type<char>, type<double>),
	MakeList(type<long long>, type<unsigned>, type<void*>, type<char*>, type<int>, type<void>, type<char>, type<double>));
//*/
//*
static auto testb = CartesianProduct(
	MakeList(type<long long>, type<unsigned>, type<void*>, type<char*>),
	MakeList(type<long long>, type<unsigned>, type<void*>, type<char*>),
	MakeList(type<long long>, type<unsigned>, type<void*>, type<char*>, type<int>, type<void>, type<char>, type<double>),
	MakeList(type<long long>, type<unsigned>, type<void*>, type<char*>, type<int>, type<void>, type<char>, type<double>),
	MakeList(0_z, 1_z, 2_z));
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
	constexpr static auto typeList_ = MakeList(type<Types>...);
	constexpr static auto typeCount_ = Size(typeList_);
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

template <class ...T> constexpr auto BoundTypes(Type<Variant<T...>>) { return ConstantList<Type<T>...>{}; }

template <class T> constexpr size_t BoundTypeIndex(const T&) { return 0; }
template <class T> constexpr auto BoundTypes(Type<T>) { return ConstantList<Type<T>>{}; }

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
			return F{}(UnsafeGetAs(A{}, static_cast<V>(v))...);
		}
	};
};

//*
struct InvokeMultiMethod_t {
	template <class F, class... V> struct Imps {
		static auto ConstantValue() {
			auto boundTypeListList = MakeList(With(Type<V&&>{}) | [](auto v) {
				return BoundTypes(Decay(v)) | [&](auto... b) {
					return MakeList(ApplyCVReference(v, b)...);
				};
			}...);
			auto getCommonResultType = MakeRecursive([&](auto getCommonResultType, auto bll, auto al) {
				return IF(Empty(bll), al | [&](auto... a) {
					return ResultType(Type<F>{}, a...);
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
				return IF(Empty(bll), VCONSTANT(&NullImp::value){}, bll | [&](auto bl0, auto... bl1) {
					auto nullImps = getNullImps(MakeList(bl1...));
					return bl0 | [&](auto... b) {
						return MakeArray(nullImps, ((void)b, nullImps)...);
					};
				});
			});
			auto getImps = MakeRecursive([&](auto getImps, auto bll, auto al) {
				return IF(Size(bll) == 0_z, al | [&](auto... a) {
					return VCONSTANT(&MultiMethodImp<F, R, V&&...>::template WithArgs<decltype(a)...>::value){};
				}, bll | [&](auto bl0, auto... bl1) {
					auto tail = MakeList(bl1...);
					return bl0 | [&](auto... b) {
						return al | [&](auto... a) {
							return MakeArray(getNullImps(tail), getImps(tail, MakeList(a..., b))...);
						};
					};
				});
			});
			return getImps(boundTypeListList, MakeList());
		}
		using ctype = decltype(ConstantValue());
		using type = decltype(ctype::Value());
		constexpr static type value = ctype::Value();
	};
	template <class F, class... V> constexpr auto operator()(F, V&&... v) const {
		auto& imps = Imps<F, V...>::value;
		auto findImp = MakeRecursive([&](const auto& callImp, const auto& imps, size_t i1, auto... i2) {
			const auto& found = imps[i1];
			return IF(ArgCount(i2...) == 0_z, found, With(found) | [&](auto& found) {
				return callImp(found, i2...);
			});
		});
		return findImp(imps, BoundTypeIndex(v)...)(static_cast<V&&>(v)...);
	}
};
template <class F, class... V>
constexpr typename InvokeMultiMethod_t::Imps<F, V...>::type InvokeMultiMethod_t::Imps<F, V...>::value;
constexpr static InvokeMultiMethod_t InvokeMultiMethod{};
//*/


/*
struct FlattenIndices_t {
	template <size_t count>
	constexpr size_t operator()(const Array<size_t, count> sizes, const Array<size_t, count>& indices) {
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
	template <class F, class... V> struct Imps {
		static auto ConstantValue() {
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

			using NullImp = MultiMethodNullImp<R, V&&...>;

			/
			auto getImpEntry = [&](auto sizes, auto il) {
				return il | [&](auto... i) {
					auto indices = MakeArray((i + 1_z)...);
					return MakeList(
						FlattenIndices(sizes, indices),
						boundTypeListList | [&](auto... bl) {
							return VCONSTANT(&MultiMethodImp<F, R, V&&...>::template WithArgs<decltype(bl[i]){}...>::value){};
						};
					);
				};
			};
			auto impEntries = boundTypeListList | [](auto... bl) {
				return With(Size(bl)...) | [](auto... s) {
					auto sizes = MakeArray(s...);
					return CartesianProduct(MakeIndexList(s)...) | [](auto... il) {
						return getImpEntry(sizes, il)...
					};
				};
			};

			//constexpr size_t impCount = Multiply(BoundTypes(Decay(type<V>)))
			//Array<R(*)(V&&...), impCount> imps;
			///

			//
			auto getBoundTypeList2 = [](auto v) {
				return BoundTypes(Decay(v)) | [&](auto... b) {
					return MakeList(Type<void>{}, ApplyCVReference(v, b)...);
				};
			};
			auto test = MakeList(getBoundTypeList2(Type<V&&>{})...) | [](auto... bl) {
				return CartesianProduct(bl...);
			};
			///

			//
			auto getNullImps = MakeRecursive([&](auto getNullImps, auto bll) {
				return IF(Empty(bll), VCONSTANT(&NullImp::value){}, bll | [&](auto bl0, auto... bl1) {
					auto nullImps = getNullImps(MakeList(bl1...));
					return bl0 | [&](auto... b) {
						return MakeArray(nullImps, ((void)b, nullImps)...);
					};
				});
			});
			auto getImps = MakeRecursive([&](auto getImps, auto bll, auto al) {
				return IF(Size(bll) == 0_z, al | [&](auto... a) {
					return VCONSTANT(&MultiMethodImp<F, R, V&&...>::template WithArgs<decltype(a)...>::value){};
				}, bll | [&](auto bl0, auto... bl1) {
					auto tail = MakeList(bl1...);
					return bl0 | [&](auto... b) {
						return al | [&](auto... a) {
							return MakeArray(getNullImps(tail), getImps(tail, MakeList(a..., b))...);
						};
					};
				});
			});
			return getImps(boundTypeListList, MakeList());
			///
		}
		using ctype = decltype(ConstantValue());
		using type = decltype(ctype::Value());
		constexpr static type value = ctype::Value();
	};
	template <class F, class... V> constexpr auto operator()(F, V&&... v) const {
		auto& imps = Imps<F, V...>::value;
		auto findImp = MakeRecursive([&](const auto& callImp, const auto& imps, size_t i1, auto... i2) {
			const auto& found = imps[i1];
			return IF(ArgCount(i2...) == 0_z, found, With(found) | [&](auto& found) {
				return callImp(found, i2...);
			});
		});
		return findImp(imps, BoundTypeIndex(v)...)(static_cast<V&&>(v)...);
	}
};
template <class F, class... V>
constexpr typename InvokeMultiMethod_t::Imps<F, V...>::type InvokeMultiMethod_t::Imps<F, V...>::value;
constexpr static InvokeMultiMethod_t InvokeMultiMethod{};
//*/

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
