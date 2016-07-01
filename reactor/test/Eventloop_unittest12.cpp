#include "../Connector.h"
#include "../EventLoop.h"

#include <stdio.h>

EventLoop* g_pLoop;

void connectCallBack(int nSockFd) {
	printf("connected.\n");
	g_pLoop->Quit();
}

int main(int argc, char* argv[]) {
	EventLoop loop;
	g_pLoop = &loop;
	InetAddress addr("127.0.0.1", 10000);
	ConnectorPtr pConnector(new Connector(&loop, addr));
	pConnector->SetNewConnectionCallBack(connectCallBack);
	pConnector->Start();

	loop.Loop();
}
