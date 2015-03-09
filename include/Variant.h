#pragma once
#include "ApplyCVReference.h"
#include "BoundTypeIndex.h"
#include "BoundTypes.h"
#include "Decay.h"
#include "FindArg.h"
#include "GetAs.h"
#include "Invalid.h"
#include "MakeList.h"
#include "Size.h"
#include "Type.h"
#include "UnsafeGetAs.h"
#include "_z.h"
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
	template <class U> constexpr static auto Tag(Type<U>) { return FindArg(type<U>, type<Types>...) + 1_z; }
	template <class U> constexpr static auto ValidType(Type<U>) { return Variant::Tag(type<U>) <= typeCount_; }
	template <class To, class From> static constexpr auto ValidGet(Type<To>, Type<From>) {
		constexpr auto to = type<To>;
		constexpr auto from = type<From>;
		constexpr auto toval = Decay(type<To>);
		constexpr auto fromval = Decay(Type<From>{});
		static_assert(type<Variant> == fromval, "Calling GetAs with wrong Variant type!");
		return Variant::ValidType(toval) && ApplyCVReference(from, toval) == to;
	}
public:
	constexpr Variant() noexcept {}
	constexpr Variant(std::nullptr_t) noexcept {}
	Variant(Variant&&) noexcept = default;
	template <class U, class V, class = std::enable_if_t<ValidType(type<V>)>>
	Variant(U&& u, Type<V> v) { data_ = { Tag(v), new V(static_cast<U&&>(u)) }; }
	template <class U> Variant(U&& u) : Variant(static_cast<U&&>(u), Decay(type<U>)) {}
	~Variant() {
		using FP = void (*)(void*);
		static const FP kFPs[] = { Noop, CastAndDelete<Types>... };
		kFPs[data_.tag](data_.buffer);
	}
	Variant& operator=(Variant _u) { swap(*this, _u); return *this; }
	friend void swap(Variant& _a, Variant& _b) { std::swap(_a.data_, _b.data_); }
	template <class U, class V>
	friend std::enable_if_t<Variant::ValidGet(type<U>, type<V&&>), U> UnsafeGetAs(Type<U>, V&& v) {
		return static_cast<U>(*static_cast<std::remove_reference_t<U>*>(v.data_.buffer));
	}
	template <class U, class V>
	friend std::enable_if_t<Variant::ValidGet(type<U>, type<V&&>), U> GetAs(Type<U>, V&& v) {
		if (v.data_.tag != Tag(Decay(type<U>)))
			throw std::bad_cast{};
		return static_cast<U>(*static_cast<std::remove_reference_t<U>*>(v.data_.buffer));
	}
	friend size_t BoundTypeIndex(const Variant& v) { return v.data_.tag; }
	friend constexpr auto BoundTypes(Type<Variant>) { return MakeList(type<InvalidNS::Invalid>, type<Types>...); }
};
