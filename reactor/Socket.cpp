#include "Socket.h"
#include "InetAddress.h"
#include "SocketOps.h"

#include "../logging/Logging.h"

#include <assert.h>
#include <strings.h> // bzero

#include <netinet/in.h>
#include <netinet/tcp.h>


Socket::Socket(int nSockFd) : m_nSockFd(nSockFd) {
	assert(nSockFd >= 0);
}

Socket::~Socket() {
	Sockets::Close(m_nSockFd);
}

int Socket::GetFd() const {
	return m_nSockFd;
}

void Socket::Bind(const InetAddress& rAddr) {
	Sockets::BindOrDie(m_nSockFd, rAddr.GetSockAddrInet());
}

void Socket::Listen() {
	Sockets::ListenOrDie(m_nSockFd);
}

int Socket::Accpet(InetAddress* pPeerAddr) {
	struct sockaddr_in addr;
	::bzero(&addr, sizeof(addr));
	int nConnFd = Sockets::Accept(m_nSockFd, addr);
	if (nConnFd >= 0) {
		pPeerAddr->SetSockAddrInet(addr);
	}
	return nConnFd;
}

void Socket::SetReuseAddr(bool bOn) {
	int nOptVal = bOn ? 1 : 0;
	if (::setsockopt(m_nSockFd, SOL_SOCKET, SO_REUSEADDR, &nOptVal, sizeof(nOptVal)) < 0) 
	{
		LOG_SYSERR << "Socket::SetReuseAddr()";
	}
}

void Socket::ShutDownWrite() {
	Sockets::ShutDownWrite(m_nSockFd);
}

void Socket::SetTcpNoDelay(bool bOn) {
	int nOptVal = bOn ? 1 : 0;
	if (::setsockopt(m_nSockFd, IPPROTO_TCP, TCP_NODELAY, &nOptVal, sizeof(nOptVal)) < 0) {
		LOG_SYSERR << "Socket::SetTcpNoDelay()";
	}
}
