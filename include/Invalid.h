#pragma once
namespace InvalidNS {
	struct Invalid {
		template <class T> operator T() const { throw ""; }
	};
}
