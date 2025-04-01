#include <iostream>
#include "SmartPointer.h"

class Base
{
public:
	virtual ~Base() {}
	virtual void action() = 0;

};

class Derived1 : public Base
{
public:
	void action()
	{
		std::cout << "perform action for Derived1" << std::endl;
	}
};

class Derived2 : public Base
{
public:
	void action()
	{
		std::cout << "perform action for Derived2" << std::endl;
	}
};

template <typename T>
using unique_ptr = MyUniquePtr<T>;

int main()
{
	unique_ptr<int> up1(new int(8));
	std::cout << *up1<<std::endl;

	unique_ptr<int> up2(new int{6});
	unique_ptr<int> up3 = nullptr;      //calls nullptr ctor //up3(nullptr)

	up3 = nullptr;                     // calls the nullptr asignment if one is provided else will call nullptr ctor

	//up3 = up2;                       //cannot copy assign from other up need to move
	up3 = std::move(up2);
	std::cout << *up3<<std::endl;

	//unique_ptr<int> up4(up3);       //cannot copy construct needs to use move semantics 
	unique_ptr<int> up4(std::move(up3));
	
	//derived class ctors
	unique_ptr<Derived1> d1(new Derived1);
	unique_ptr<Derived2> d2(new Derived2);
	unique_ptr<Derived2> d3(new Derived2);

	unique_ptr<Base> base1 = std::move(d1);   //calls derived move ctor
	unique_ptr<Base> base2(std::move(d2));   
	unique_ptr<Base> base3;

	base3 = std::move(d3);                   //calls derived move assigment

	base1->action();
	base2->action();
	base3->action();
	
        getchar();
	return 0;

}

