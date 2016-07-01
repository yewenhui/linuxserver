#include "../BlockingQueue.h"
#include "../CountDownLatch.h"
#include "../Thread.h"

#include <string>
#include <stdio.h>
#include <assert.h>

#include <boost/bind.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

class Test
{
public:
	Test(int nNumThreads)
		: m_Latch(nNumThreads)
		, m_vThread(nNumThreads)
	{
		for (int n = 0; n < nNumThreads; ++n) {
			char name[32];
			snprintf(name, sizeof(name), "work thread %d", n);
			m_vThread.push_back(new Thread(boost::bind(&Test::threadFunc, this), std::string(name)));
		}
		for_each(m_vThread.begin(), m_vThread.end(), boost::bind(&Thread::Start, _1));
	}

	void run(int nTimes) {
		if (nTimes <= 0) {
			assert(false);
			return;
		}

		 printf("waiting for count down latch\n");
		 m_Latch.Wait();
		 printf("all threads started\n");
		 for (int n = 0; n < nTimes; ++ n) {
			 char buf[32];
			 snprintf(buf, sizeof(buf), "hello %d", n);
			 m_Queue.Put(buf);
			 printf("tid = %d, put data = %s, size = %zd\n"
				 , CurrentThread::GetTid(), buf, m_Queue.Size());
		 }
	}

	void joinAll() {
		for (size_t n = 0; n < m_vThread.size(); ++n) {
			m_Queue.Put("stop");
		}
		for_each(m_vThread.begin(), m_vThread.end(), boost::bind(&Thread::Join, _1));
	}

private:
	void threadFunc() {
		printf("tid = %d, %s started\n", CurrentThread::GetTid(), CurrentThread::Name());
		m_Latch.CountDown();
		bool bRunning = true;
		while (bRunning) {
			std::string str(m_Queue.Take());
			printf("tid = %d, get data = %s, size = %zd\n", CurrentThread::GetTid()
				, str.c_str(), m_Queue.Size());

			 bRunning = (str != "stop");
		}

		printf("tid = %d, %s stoped\n", CurrentThread::GetTid(), CurrentThread::Name());
	}

private:
	BlockingQueue<std::string>	m_Queue;
	CountDownLatch				m_Latch;
	boost::ptr_vector<Thread>	m_vThread;

};

int main()
{
	printf("pid = %d, tid = %d\n", ::getpid(), CurrentThread::GetTid());
	Test t(5);
	t.run(100);
	t.joinAll();

	printf("number of created threads %d\n", Thread::GetNumCreated());
}
