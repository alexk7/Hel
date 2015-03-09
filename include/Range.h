#pragma once
#include "Begin.h"
#include "End.h"
template <class T, class U> class Range {
	T begin_;
	U end_;
public:
	template <class B, class E> Range(B&& b, E&& e) : begin_(b), end_(e) {}
	friend constexpr T Begin(const Range& r) { return r.begin_; }
	friend constexpr U End(const Range& r) { return r.end_; }
};
