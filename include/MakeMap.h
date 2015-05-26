#pragma once
#include "ConstantList.h"
#include "ConstantMap.h"
template <class... K, class... V> auto MakeMap(ConstantList<K, V>...) { return ConstantMap<ConstantList<K, V>...>{}; }
