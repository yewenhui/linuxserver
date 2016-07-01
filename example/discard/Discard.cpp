#include "Discard.h"

#include <Logging.h>

#include <boost/bind.hpp>

DiscardServer::DiscardServer(EventLoop* pEvtLoop, const InetAddress& listenAddr) 
	: m_Server(pEvtLoop, listenAddr, "DiscardServer")
{
	m_Server.SetConnectionCallBack(boost::bind(&DiscardServer::onConnection, this, _1));	
	m_Server.SetMessageCallBack(boost::bind(&DiscardServer::onMessage, this, _1, _2, _3));
}

void DiscardServer::Start() {
	m_Server.Start();
}

void DiscardServer::onConnection(const TcpConnectionPtr& pTcpConn) {
	LOG_INFO << "DiscardServer - " << pTcpConn->GetPeerAddress().ToIpPort() << " -> "
				<< pTcpConn->GetLocalAddress().ToIpPort() << " is "
				<< (pTcpConn->IsConnected() ? "IP" : "DOWN");
}

void DiscardServer::onMessage(const TcpConnectionPtr& pTcpConn, Buffer* pBuf, TimeStamp time) {
	string msg(pBuf->RetrieveAsString());
	LOG_INFO << pTcpConn->GetName() << " discards " << msg.size()
		<< " bytes received at " << time.ToString();
}

