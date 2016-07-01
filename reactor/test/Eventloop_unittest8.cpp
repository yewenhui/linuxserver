#include "../CallBacks.h"
#include "../TcpServer.h"
#include "../EventLoop.h"
#include <stdio.h>

void onConnection(const TcpConnectionPtr& pTcpConn) {
	if (pTcpConn->IsConnected()) {
		printf("onConnection(): tid=%d new connection [%s] from %s\n"
			, CurrentThread::GetTid()
			, pTcpConn->GetName().c_str()
			, pTcpConn->GetPeerAddress().ToHostPort().c_str());
	} else {
		printf("onConnection(): tid=%d connection [%s] is down\n", CurrentThread::GetTid(), pTcpConn->GetName().c_str());
	}
}

void onMessage(const TcpConnectionPtr& pTcpConn, Buffer* buffer, TimeStamp reveiveTime) {
	printf("onMessage(): tid=%d received %zd bytes from connection [%s] at %s    \n",
	       CurrentThread::GetTid()
		   , buffer->ReadableBytes()
		   , pTcpConn->GetName().c_str()
		   , reveiveTime.ToFormattedString().c_str());
	printf("onMessage(): [%s]\n", buffer->RetrieveAsString().c_str());
}

int main(int argc, char* argv[]) {
	EventLoop loop;
	InetAddress listenAddr(10000);
	TcpServer server(&loop, listenAddr);
	server.SetConnectionCallBack(onConnection);
	server.SetMessageCallBack(onMessage);
	if (argc > 1) {
		server.SetThreadNum(atoi(argv[1]));
	}
	server.Start();
	loop.Loop();
}
