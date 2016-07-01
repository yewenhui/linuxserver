#include "../EventLoop.h"

#include <stdio.h>

#include <boost/bind.hpp>

int nCnt = 0;
EventLoop* g_pEventLoop;

void printTid() {
	printf("pid = %d, tid = %d\n", getpid(), CurrentThread::GetTid());
	printf("now %s\n", TimeStamp::Now().ToFormattedString().c_str());
}

void print(const char* pStrMsg) {
	printf("msg %s %s\n", TimeStamp::Now().ToFormattedString().c_str(), pStrMsg);
	if (++nCnt == 20) {
		g_pEventLoop->Quit();
	}
}

TimerId toCancel;
void cancelSelf() {
	printf("cancelSelf()\n");
	g_pEventLoop->Cancel(toCancel);
}

int main() {
	printTid();

	EventLoop loop;
	g_pEventLoop = &loop;

	print("main");
	g_pEventLoop->RunAfter(1, boost::bind(print, "once1"));
	g_pEventLoop->RunAfter(1.5, boost::bind(print, "once1.5"));
	g_pEventLoop->RunAfter(2.5, boost::bind(print, "once2.5"));
	g_pEventLoop->RunAfter(3.5, boost::bind(print, "once3.5"));
	//g_pEventLoop->RunEvery(2, boost::bind(print, "every2"));
	TimerId t = loop.RunEvery(2, boost::bind(print, "every2"));
	g_pEventLoop->RunEvery(3, boost::bind(print, "every3"));
	loop.RunAfter(10, boost::bind(&EventLoop::Cancel, &loop, t));
	toCancel = loop.RunEvery(5, cancelSelf);

	g_pEventLoop->Loop();
	print("main loop exits");
	::sleep(1);
}
