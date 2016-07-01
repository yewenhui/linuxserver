#include "../ThreadLocal.h"
#include "../Thread.h"

#include <boost/noncopyable.hpp>
#include <stdio.h>

class Test
	: private boost::noncopyable
{
public:
	Test() {
		printf("tid = %d, constructing %p\n", CurrentThread::GetTid(), this);
	}

	~Test() {
		printf("tid = %d, destructing %p %s\n", CurrentThread::GetTid(), this, m_strName.c_str());
	}

	const std::string& name() const { return m_strName; }
	void setName(const std::string& n) { m_strName = n; }

private:
	std::string			m_strName;

};

ThreadLocal<Test> testObj1;
ThreadLocal<Test> testObj2;

void print()
{
	printf("tid = %d, obj1 %p name = %s\n"
		, CurrentThread::GetTid()
		, &testObj1.Value()
		, testObj1.Value().name().c_str());
	printf("tid = %d, obj2 %p name = %s\n"
		, CurrentThread::GetTid()
		, &testObj2.Value()
		, testObj2.Value().name().c_str());
}

void threadFunc()
{
	print();
	testObj1.Value().setName("changed 1");
	testObj2.Value().setName("changed 42");
	print();
}

int main()
{
	testObj1.Value().setName("main one");
	print();

	Thread t1(threadFunc);
	t1.Start();
	t1.Join();
	testObj2.Value().setName("main two");
	print();

	pthread_exit(0);
}
