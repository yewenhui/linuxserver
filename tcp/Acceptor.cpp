#include "Acceptor.h"
#include "TcpStream.h"

#include <utility>
#include <stdio.h>
#include <sys/socket.h>

Acceptor::Acceptor(const InetAddress& rListenAddr)
	: m_ListenSocket(Socket::CreateTcp())
{
	m_ListenSocket.SetReuseAddr(true);
	m_ListenSocket.Bind(rListenAddr);
	m_ListenSocket.Listen();
}

TcpStreamPtr Acceptor::Accept() {
	int nSockFd = ::accept(m_ListenSocket.GetFd(), NULL, NULL);
	if (nSockFd >= 0) {
		return TcpStreamPtr(new TcpStream(std::move(Socket(nSockFd))));
	}
	return TcpStreamPtr();
}
