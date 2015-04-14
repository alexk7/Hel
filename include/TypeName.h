#pragma once
#include "MakeUniquePtr.h"
#include "Type.h"
#include <cxxabi.h>
#include <typeinfo>
#include <string>
#include <stdlib.h>
#include <regex>
template <class T> std::string TypeName(Type<T>) {
	std::string name = MakeUniquePtr(abi::__cxa_demangle(typeid(T).name(), 0, 0, 0), free).get();
	name = std::regex_replace(name, std::regex(" >, "), ">,\n\t");
	return name;
}
