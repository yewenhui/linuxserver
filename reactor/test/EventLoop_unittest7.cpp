#include "../Acceptor.h"
#include "../EventLoop.h"
#include "../InetAddress.h"
#include "../SocketOps.h"

#include <stdio.h>

void newConnection(int nSockFd, const InetAddress& rPeerAddr) {
	printf("Accpected a new connection from %s\n", rPeerAddr.ToIpPort().c_str());
	::write(nSockFd, "How are you?\n", 13);
	Sockets::Close(nSockFd);
}

int main() {
	InetAddress listenAddr(10000);
	EventLoop loop;

	Acceptor acceptor(&loop, listenAddr);
	acceptor.SetNewConnectionCallback(newConnection);
	acceptor.Listen();

	loop.Loop();
}
