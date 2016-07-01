#include "../EventLoop.h"
#include "../InetAddress.h"
#include "../TcpClient.h"

#include <stdio.h>
#include <unistd.h>

std::string strMsg = "Hello\n";

void onConnection(const TcpConnectionPtr& pTcpConn) {
	if (pTcpConn->IsConnected()) {
		printf("onConnection(): new connection [%s] from %s\n"
			, pTcpConn->GetName().c_str()
			, pTcpConn->GetPeerAddress().ToHostPort().c_str());
		pTcpConn->Send(strMsg);
	} else {
		printf("onConnection(): connection [%s] is down\n", pTcpConn->GetName().c_str());
	}
}

void onMessage(const TcpConnectionPtr& pTcpConn, Buffer* pBuf, TimeStamp receiveTime) {
	printf("onMessage(): received %zd bytes from connection [%s] at %s    \n"
		, pBuf->ReadableBytes()
		, pTcpConn->GetName().c_str()
		, receiveTime.ToFormattedString().c_str());

	printf("onMessage(): [%s]\n", pBuf->RetrieveAsString().c_str());
}
int main() {
	EventLoop loop;
	InetAddress serverAddr("localhost", 10000);
	TcpClient client(&loop, serverAddr);

	client.SetConnectionCallBack(onConnection);
	client.SetMessageCallBack(onMessage);
	client.EnableRetry();
	client.Connect();

	loop.Loop();

}