#pragma once
namespace InvalidNS {
	struct Invalid {
		template <class T> [[noreturn]] operator T() const { throw; } //This should be optimized away and never called.
	};
}
