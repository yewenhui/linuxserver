#include "../EventLoop.h"
#include "../EventLoopThread.h"

#include <stdio.h>

void runInThread() {
	printf("runInThread(): pid = %d, tid = %d\n", ::getpid(), CurrentThread::GetTid());
}

int main() {
	printf("main(): pid = %d, tid = %d\n", ::getpid(), CurrentThread::GetTid());

	EventLoopThread evtLoopThread;
	EventLoop* pLoop = evtLoopThread.StartLoop();
	pLoop->RunInLoop(runInThread);
	::sleep(1);
	pLoop->RunAfter(2, runInThread);
	::sleep(2);
	pLoop->Quit();

	printf("exit main().\n");
}
