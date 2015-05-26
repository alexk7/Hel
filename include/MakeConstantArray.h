#pragma once
#include "ArgCount.h"
#include "MakeIndexList.h"
#include "MakeList.h"
#include "MakeMap.h"
static auto MakeConstantArray = [](auto... c) {
	return MakeIndexList(ArgCount(c...)) | [c...](auto... i) {
		return MakeMap(MakeList(i, c)...);
	};
};
