#include "../Thread.h"

#include <string>
#include <stdio.h>
#include <boost/bind.hpp>

void threadFunc() {
	printf("tid = %d\n", CurrentThread::GetTid());
}

void threadFunc2(int x) {
	printf("tid = %d, x = %d\n", CurrentThread::GetTid(), x);
}

void threadFunc3() {
	printf("tid = %d\n", CurrentThread::GetTid());
	sleep(1);
}

class Foo
{
public:
	explicit Foo(double dVal)
		: m_dVal(dVal)
	{
	}
	
	void memberFunc() {
		printf("tid = %d, Foo:m_dVal = %f\n", CurrentThread::GetTid(), m_dVal);
	}

	void memberFunc2(const std::string& strText) {
		printf("tid = %d, Foo:m_dVal = %f, text = %s\n", CurrentThread::GetTid(), m_dVal, strText.c_str());
	}

private:
	double		m_dVal;

};

int main()
{
	printf("pid = %d, tid = %d\n", ::getpid(), CurrentThread::GetTid());

	Thread t1(threadFunc);
	t1.Start();
	t1.Join();

	Thread t2(boost::bind(threadFunc2, 42), "thread for free function with argument");
	t2.Start();
	t2.Join();

	Foo foo(87.43);
	Thread t3(boost::bind(&Foo::memberFunc, &foo), "thread for member function without argument");
	t3.Start();
	t3.Join();

	Thread t4(boost::bind(&Foo::memberFunc2, boost::ref(foo), std::string("LeonYe")));
	t4.Start();
	t4.Join();

	{
		Thread t5(threadFunc3);
		t5.Start();
	}
	sleep(2);
	printf("number of created threads %d\n", Thread::GetNumCreated());

	return 0;
}
