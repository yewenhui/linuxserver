#include "../EventLoop.h"

#include <stdio.h>

EventLoop* g_pEvtloop;
int g_nFlag = 0;

void run4() {
	printf("run4(): pid = %d, flag = %d\n", ::getpid(), g_nFlag);
	g_pEvtloop->Quit();
}

void run3() {
	printf("run3(): pid = %d, flag = %d\n", ::getpid(), g_nFlag);
	g_pEvtloop->RunAfter(3, run4);
	g_nFlag = 3;
}

void run2() {
	printf("run2(): pid = %d, flag = %d\n", ::getpid(), g_nFlag);
	g_pEvtloop->QueueInLoop(run3);
}

void run1() {
	g_nFlag = 1;
	printf("run1(): pid = %d, flag = %d\n", ::getpid(), g_nFlag);
	g_pEvtloop->RunInLoop(run2);
	g_nFlag = 2;
}

int main() {
	printf("main(): pid = %d, flag = %d\n", ::getpid(), g_nFlag);

	EventLoop evtLoop;
	g_pEvtloop = &evtLoop;

	evtLoop.RunAfter(2, run1);
	g_pEvtloop->Loop();

	printf("main(): pid = %d, flag = %d\n", ::getpid(), g_nFlag);
}
