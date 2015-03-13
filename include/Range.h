#pragma once
template <class T, class U> class Range {
	T begin_;
	U end_;
public:
	template <class B, class E> Range(B&& b, E&& e) : begin_(b), end_(e) {}
	friend constexpr T begin(const Range& r) { return r.begin_; }
	friend constexpr T end(const Range& r) { return r.end_; }
	friend constexpr auto empty(const Range& r) { return r.begin_ == r.end_; }
};
