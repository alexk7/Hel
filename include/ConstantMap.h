#pragma once
#include "ConstantList.h"
template <class... KV> class ConstantMap {
	struct KVHack : KV... {};
	template <class K, class V> static V VHack(ConstantList<K, V>); //undefined
public:
	template <class K> auto operator[](K) const { return decltype(VHack<K>(KVHack{})){}; }
};
