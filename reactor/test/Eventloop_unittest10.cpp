#include "../TcpServer.h"
#include "../EventLoop.h"
#include "../InetAddress.h"

#include <stdio.h>

std::string strMsg1;
std::string strMsg2;
int nSleepSeconds = 0;

void onConnection(const TcpConnectionPtr& pTcpConn) {
	if (pTcpConn->IsConnected()) {
		printf("tid=%d new connection [%s] from %s\n", CurrentThread::GetTid(), pTcpConn->GetName().c_str(), pTcpConn->GetPeerAddress().ToHostPort().c_str());
		if (nSleepSeconds > 0) {
			::sleep(nSleepSeconds);
		}
		if (!strMsg1.empty()) {
			pTcpConn->Send(strMsg1);
		}
		if (!strMsg2.empty()) {
			pTcpConn->Send(strMsg2);
		}
		
		pTcpConn->ShutDown();
	} else {
		printf("connection [%s] is down\n", pTcpConn->GetName().c_str());
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

int main(int argc, char* argv[]) {
	int nLen1 = 100;
	int nLen2 = 100;
	if (argc > 2) {
		nLen1 = atoi(argv[1]);
		nLen2 = atoi(argv[2]);
	}
	if (argc > 3) {
		nSleepSeconds = atoi(argv[3]);
	}

	strMsg1.resize(nLen1);
	strMsg2.resize(nLen2);

	std::fill(strMsg1.begin(), strMsg1.end(), 'A');
	std::fill(strMsg2.begin(), strMsg2.end(), 'B');

	InetAddress listenAddr(10000);
	EventLoop loop;

	TcpServer server(&loop, listenAddr);
	server.SetConnectionCallBack(onConnection);
	server.SetMessageCallBack(onMessage);
	if (argc > 3) {
		server.SetThreadNum(atoi(argv[3]));
	}
	server.Start();

	loop.Loop();

}


