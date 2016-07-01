#include "Acceptor.h"
#include "EventLoop.h"
#include "SocketOps.h"
#include "InetAddress.h"

#include <utility>
#include <stdio.h>
#include <sys/socket.h>

#include <boost/bind.hpp>

Acceptor::Acceptor(EventLoop* pEventLoop, const InetAddress& rListenAddr)
	: m_pEventLoop(pEventLoop)
	, m_AccpetSocket(Sockets::CreateNonBlockingOrDie())
	, m_AcceptChannel(m_pEventLoop, m_AccpetSocket.GetFd())
{
	m_AccpetSocket.SetReuseAddr(true);
	m_AccpetSocket.Bind(rListenAddr);
	m_AcceptChannel.SetReadCallBack(boost::bind(&Acceptor::handleRead, this));
}

void Acceptor::Listen() {
	m_pEventLoop->AssertInLoopThread();
	m_bListenning = true;
	m_AccpetSocket.Listen();
	m_AcceptChannel.EnableReading();
}

void Acceptor::handleRead() {
	m_pEventLoop->AssertInLoopThread();
	InetAddress peerAddr(0);
	int nConnFd = m_AccpetSocket.Accpet(&peerAddr);
	if (nConnFd >= 0) {
		if (m_NewConCallBack) {
			m_NewConCallBack(nConnFd, peerAddr);
		} else {
			Sockets::Close(nConnFd);
		}
	}
}
