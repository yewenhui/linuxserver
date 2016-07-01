#include "../TcpServer.h"
#include "../EventLoop.h"
#include "../InetAddress.h"

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

void onMessage(const TcpConnectionPtr& pTcpConn, Buffer* pBuf, TimeStamp receiveTime) {
	printf("onMessage(): tid=%d received %zd bytes from connection [%s] at %s    \n",
		CurrentThread::GetTid()
		, pBuf->ReadableBytes()
		, pTcpConn->GetName().c_str()
		, receiveTime.ToFormattedString().c_str());
	printf("onMessage(): [%s]\n", pBuf->RetrieveAsString().c_str());
}

int main(int argc, char* argv[]) {
	InetAddress listenAddr(10000);
	EventLoop loop;

	TcpServer server(&loop, listenAddr);
	server.SetConnectionCallBack(onConnection);
	server.SetMessageCallBack(onMessage);
	if (argc > 1) {
		server.SetThreadNum(atoi(argv[1]));
	}
	server.Start();

	loop.Loop();
}
