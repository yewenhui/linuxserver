#include "../EventLoop.h"
#include "../../thread/Thread.h"

EventLoop* g_pLoop = NULL;

void threadFunc() {
	g_pLoop->Loop();
}

int main() {
	EventLoop loop;
	g_pLoop = &loop;
	Thread t(threadFunc);

	t.Start();
	t.Join();
}
