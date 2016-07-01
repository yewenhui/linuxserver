#include "../ThreadPool.h"
#include "../CountDownLatch.h"

#include <boost/bind.hpp>
#include <stdio.h>

void print() {
	printf("tid = %d\n", CurrentThread::GetTid());
}

void printString(const std::string& strName) {
	printf("tid = %d, str = %s\n", CurrentThread::GetTid(), strName.c_str());
}

int main()
{
	ThreadPool threadPool("MainThreadPool");
	threadPool.Start(5);

	threadPool.Run(print);
	threadPool.Run(print);

	for (int n = 0; n < 100; ++n) {
		char buf[32];
		snprintf(buf, sizeof(buf), "task %d", n);
		threadPool.Run(boost::bind(printString, std::string(buf)));
	}
	
	CountDownLatch latch(1);
	threadPool.Run(boost::bind(&CountDownLatch::CountDown, &latch));
	latch.Wait();
	threadPool.Stop();
}
