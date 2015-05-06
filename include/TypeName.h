#pragma once
#include "MakeUniquePtr.h"
#include <cxxabi.h>
#include <typeinfo>
#include <string>
#include <stdlib.h>
#include <regex>
inline std::string TypeName(const std::type_info& t) {
	std::string name = MakeUniquePtr(abi::__cxa_demangle(t.name(), 0, 0, 0), free).get();
	name = std::regex_replace(name, std::regex(" >, "), ">,\n\t"); //easier to read, TODO: accumulate indent level
	name = std::regex_replace(name, std::regex("::__1"), ""); //for libc++
	//TODO: remove default stl allocator (all default arguments?)
	return name;
}
