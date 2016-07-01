#include "../Mutex.h"

#include <vector>
#include <stdio.h>

#include <boost/shared_ptr.hpp>

class Foo
{
public:
	void		doIt() const;

};

typedef std::vector<Foo> FooList;
typedef boost::shared_ptr<FooList> FooListPtr;

FooListPtr pFooList;
MutexLock mutex;

void post(const Foo& rFoo) {
	printf("post() \n");
	MutexLockGuard lock(mutex);
	if (!pFooList.unique()) {
		pFooList.reset(new FooList(*pFooList));
		printf("copy the whole list\n");
	}
	assert(pFooList.unique());
	pFooList->push_back(rFoo);

}

void traverse() {
	FooListPtr pFoos;
	{
		// 这里是智能指针赋值需要锁的，需要注意
		MutexLockGuard lock(mutex);
		pFoos = pFooList;
		assert(!pFooList.unique());
	}

	assert(!pFoos.unique());
	for (std::vector<Foo>::const_iterator itor = pFoos->begin()
		; itor != pFoos->end()
		; ++itor)
	{
		itor->doIt();
	}
}

void Foo::doIt() const {
	Foo f;
	post(f);
}

int main()
{
	pFooList.reset(new FooList());
	Foo foo;
	post(foo);
	traverse();
}
