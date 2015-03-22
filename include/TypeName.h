#pragma once
#include "MakeUniquePtr.h"
#include "Type.h"
#include <cxxabi.h>
#include <typeinfo>
#include <string>
#include <stdlib.h>
template <class T> std::string TypeName(Type<T>) {
	return MakeUniquePtr(abi::__cxa_demangle(typeid(T).name(), 0, 0, 0), free).get();
}
