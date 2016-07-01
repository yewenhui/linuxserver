#ifndef _DISCARD_H_
#define _DISCARD_H_

#include <TcpServer.h>

class DiscardServer
{
public:
	DiscardServer(EventLoop* pEvtLoop, const InetAddress& listenAddr);

	void		Start();

private:
	void		onConnection(const TcpConnectionPtr& pTcpConn);
	void		onMessage(const TcpConnectionPtr& pTcpConn, Buffer* pBuf, TimeStamp time);

private:
	TcpServer	m_Server;

};

#endif // _DISCARD_H_
