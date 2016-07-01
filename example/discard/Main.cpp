#include "Discard.h"

#include <Logging.h>
#include <EventLoop.h>

int main() {
	LOG_INFO << "pid = " << ::getpid();
	EventLoop loop;
	InetAddress listenAddr(10000);
	DiscardServer server(&loop, listenAddr);
	server.Start();
	loop.Loop();

}

