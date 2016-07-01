#include "../Singleton.h"
#include "../Thread.h"

#include <boost/noncopyable.hpp>
#include <stdio.h>
#include <unistd.h>

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

void threadFunc() {
	printf("tid = %d, %p name = %s\n"
		, CurrentThread::GetTid()
		, &Singleton<Test>::Instance()
		, Singleton<Test>::Instance().name().c_str());

	Singleton<Test>::Instance().setName("only one, changed");
}

template<unsigned N>
class Destruct
{
public:
	Destruct() {
		printf("construct %d\n", N);
		more();
	}

	~Destruct() {
		printf("destruct %d\n", N);
	}

	void more() {
		Singleton<Destruct<N - 1> >::Instance();
	}
};

template<>
void Destruct<0>::more() {

}

int main()
{
	Singleton<Test>::Instance().setName("only one");
	Thread t1(threadFunc);
	t1.Start();
	t1.Join();

	printf("tid = %d, %p name = %s\n"
		, CurrentThread::GetTid()
		, &Singleton<Test>::Instance()
		, Singleton<Test>::Instance().name().c_str());

	Singleton<Destruct<100> >::Instance();
	printf("ATEXIT_MAX = %ld\n", sysconf(_SC_ATEXIT_MAX));
}
