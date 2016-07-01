#include "../TcpServer.h"
#include "../EventLoop.h"
#include "../InetAddress.h"

#include <stdio.h>

std::string strMsg;

void onConnection(const TcpConnectionPtr& pTcpConn) {
	if (pTcpConn->IsConnected()) {
		printf("onConnection(): tid=%d new connection [%s] from %s\n"
			, CurrentThread::GetTid()
			, pTcpConn->GetName().c_str()
			, pTcpConn->GetPeerAddress().ToHostPort().c_str());
		pTcpConn->Send(strMsg);
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

	pBuf->RetrieveAll();
}

void onWriteComplete(const TcpConnectionPtr& pTcpConn) {
	// pTcpConn->Send(strMsg);
}

int main(int argc, char* argv[]) {
	std::string line;
	for (int i = 33; i < 127; ++i) {
		line.push_back(char(i));
	}
	line += line;
	//strMsg += line;

	for (size_t i = 0; i < 127-33; ++i) {
		strMsg += line.substr(i, 72) + '\n';
	}

	InetAddress listenAddr(10000);
	EventLoop loop;

	TcpServer server(&loop, listenAddr);
	server.SetConnectionCallBack(onConnection);
	server.SetMessageCallBack(onMessage);
	server.SetWriteCompleteCallBack(onWriteComplete);
	if (argc > 1) {
		server.SetThreadNum(atoi(argv[1]));
	}
	server.Start();

	loop.Loop();
}
