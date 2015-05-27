#include "DEFINE_MULTIMETHOD.h"
#include "Type.h"
#include "Variant.h"
#include <iostream>
#include <stdio.h>
#define DTOR_TEST_IMPL
//#define DTOR_TEST_IMPL cout << __PRETTY_FUNCTION__ << endl;
DEFINE_MULTIMETHOD(Print)
DEFINE_MULTIMETHOD(Index)
DEFINE_MULTIMETHOD(CIndex)
DEFINE_MULTIMETHOD(Intersect)
DEFINE_MULTIMETHOD(Copy)
DEFINE_MULTIMETHOD(Test3)
struct Circle { ~Circle() { DTOR_TEST_IMPL } };
struct Rectangle { ~Rectangle() { DTOR_TEST_IMPL } };
struct Triangle { ~Triangle() { DTOR_TEST_IMPL } };
struct S1 { ~S1() { DTOR_TEST_IMPL } };
struct S2 { ~S2() { DTOR_TEST_IMPL } };
struct S3 { ~S3() { DTOR_TEST_IMPL } };
struct S4 { ~S4() { DTOR_TEST_IMPL } };
struct S5 { ~S5() { DTOR_TEST_IMPL } };
struct S6 { ~S6() { DTOR_TEST_IMPL } };
struct S7 { ~S7() { DTOR_TEST_IMPL } };
struct S8 { ~S8() { DTOR_TEST_IMPL } };
struct S9 { ~S9() { DTOR_TEST_IMPL } };
struct S10 { ~S10() { DTOR_TEST_IMPL } };
using Shape2 = Variant<Circle, Rectangle>;
using Shape3 = Variant<Circle, Rectangle, S1, S2, S3, S4, S5, S6, S7, S8, S9, S10, Triangle>;
using Shape3b = Variant<Circle, Rectangle, Triangle>;
inline bool Intersect(Circle, Rectangle) {
	puts(__PRETTY_FUNCTION__);
	return false;
}
inline bool Intersect(const Rectangle&, Circle) {
	puts(__PRETTY_FUNCTION__);
	return false;
}
inline bool Intersect(Circle, Circle) {
	puts(__PRETTY_FUNCTION__);
	return true;
}
inline bool Intersect(Rectangle, Rectangle) {
	puts(__PRETTY_FUNCTION__);
	return true;
}
inline bool Intersect(Rectangle&&, Circle) {
	puts(__PRETTY_FUNCTION__);
	return false;
}
inline bool Intersect(const Triangle&, const Triangle&) {
	puts(__PRETTY_FUNCTION__);
	return false;
}
template <class T>
inline bool Intersect(const Triangle&, const T&) {
	puts(__PRETTY_FUNCTION__);
	return false;
}
template <class T>
inline bool Intersect(const T&, const Triangle&) {
	puts(__PRETTY_FUNCTION__);
	return false;
}
inline void Print(S1) {
	puts(__PRETTY_FUNCTION__);
}
inline void Print(S2) {
	puts(__PRETTY_FUNCTION__);
}
inline void Print(S3) {
	puts(__PRETTY_FUNCTION__);
}
inline void Print(S4) {
	puts(__PRETTY_FUNCTION__);
}
inline void Print(S5) {
	puts(__PRETTY_FUNCTION__);
}
inline void Print(S6) {
	puts(__PRETTY_FUNCTION__);
}
inline void Print(S7) {
	puts(__PRETTY_FUNCTION__);
}
inline void Print(S8) {
	puts(__PRETTY_FUNCTION__);
}
inline void Print(S9) {
	puts(__PRETTY_FUNCTION__);
}
inline void Print(S10) {
	puts(__PRETTY_FUNCTION__);
}
inline void Print(Circle) {
	puts(__PRETTY_FUNCTION__);
}
inline void Print(Rectangle) {
	puts(__PRETTY_FUNCTION__);
}
inline void Print(Triangle) {
	puts(__PRETTY_FUNCTION__);
}
inline int Index(const Circle&) {
	puts(__PRETTY_FUNCTION__);
	return -1;
}
inline unsigned Index(Circle&&) {
	puts(__PRETTY_FUNCTION__);
	return 1;
}
inline long Index(Rectangle) {
	puts(__PRETTY_FUNCTION__);
	return 2;
}
inline long long Index(Triangle) {
	puts(__PRETTY_FUNCTION__);
	return 0;
}
inline Circle Copy(Circle c) {
	puts(__PRETTY_FUNCTION__);
	return c;
}
inline Rectangle Copy(Rectangle r) {
	puts(__PRETTY_FUNCTION__);
	return r;
}
inline Triangle Copy(Triangle t) {
	puts(__PRETTY_FUNCTION__);
	return t;
}
constexpr int CIndex(const Circle&) {
	return 0;
}
constexpr int CIndex(const Rectangle&) {
	return 1;
}
constexpr int CIndex(const Triangle&) {
	return 2;
}
using namespace std;
inline void Test3(const Shape3&, const Shape3&, const Shape3&) {
	puts(__PRETTY_FUNCTION__);
}
inline void Test3(Circle, Rectangle, Triangle) {
	puts(__PRETTY_FUNCTION__);
}
int main()
{
	//cout << Name(Type<decltype(testb)>{}) << endl;
	/* not supported yet
	{
		constexpr Shape3 s{Circle{}};
		constexpr int cindex = CIndex(s);
	}
	//*/
#if 1
	{
		Shape3 s;
		s = Circle{};
		Print(s);
	}
#endif
#if 1
	{
		Shape2 v, v2, v3;
		v = Circle{};
		v2 = Rectangle{};
		cout << Intersect(v, v) << endl;
		cout << Intersect(v, v2) << endl;
		cout << Intersect(v2, v2) << endl;
		cout << Intersect(v2, v) << endl;
		cout << Intersect(move(v2), v) << endl;
	}
#endif
#if 0
	{
		Shape3 v, v2, v3;
		v = Circle{};
		v2 = Rectangle{};
		v3 = Triangle{};
		Test3(v, v, v);
		Test3(v, v2, v3);
	}
#endif
#if 1
	{
		Shape3b v, v2, v3;
		v = Circle{};
		v2 = Rectangle{};
		v3 = Triangle{};
		Test3(v, v, v);
		Test3(v, v2, v3);
	}
#endif
#if 1
	Circle c;
	Rectangle r;
	{
		using Shape = Variant<Triangle, Circle, Rectangle>;
		Shape s{c};
		Print(s);
		Shape s2;
		s2 = c;
		Shape s3{r};
		Shape s4{nullptr};
		//Print(s4);
		//Shape s5{"allo"};
		//Shape s6{0}; //delegating ctor disable 0 -> nullptr conversion
		//Shape s6_1{1};
		Shape s7{Triangle{}};
		Shape s8{move(s)};
		//Shape s8_1{double{}};
		Shape s9;
		char c = getchar();
		if (c == 'c')
			s9 = Circle{};
		else if (c == 'r')
			s9 = Rectangle{};
		else if (c == 't')
			s9 = Triangle{};
		try {
			Intersect(s9, s2);
			Print(s9);
			cout << Index(s9) << endl;
			cout << Index(move(s9)) << endl;
		} catch (const exception& e) {
			puts(e.what());
		}
		cout << "----------" << endl;
		s7 = move(s3);
		cout << "----------" << endl;
	}
	cout << "----------" << endl;
#endif
}
