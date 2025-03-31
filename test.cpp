#include <iostream>
#include "SmartPointer.h"

using std::endl;
using std::cout;

struct MyStruct
{
	MyStruct() = default;
	MyStruct(int a, int b) :a(a), b(b) {}
	int a;
	int b;
};

int main()
{
        MyStruct *s = new MyStruct();
	s->a = 10;
	s->b = 20;

	MySharedPtr<MyStruct> sp(s);

	cout << sp->a << endl;
	cout << sp->b << endl;
	cout << (*sp).a << endl;

	auto sp2 = make_my_shared<MyStruct>(100, 200);
	cout << sp2->a << endl;
	cout << sp2->b << endl;

	auto p = sp2.release();
	cout << p->a << endl;
	cout << p->b << endl;
	delete p;
	return 0;
}
