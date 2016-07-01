#include "../EventLoop.h"

#include <stdio.h>

void threadFunc() {
	printf("threadFunc(): pid = %d, tid = %d\n", getpid(), CurrentThread::GetTid());

	EventLoop loop;
	loop.Loop();
}

int main() {
	printf("threadFunc(): pid = %d, tid = %d\n", getpid(), CurrentThread::GetTid());
	EventLoop loop;

	Thread thread(threadFunc);
	thread.Start();

	loop.Loop();
	// pthread_exit(NULL); // ±‡“Î–Îº” -pthread
}
