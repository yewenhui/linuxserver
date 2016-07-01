#include "Socket.h"
#include "InetAddress.h"

#include <assert.h>
#include <strings.h>

#include <netinet/in.h>
#include <netinet/tcp.h>

#include <boost/implicit_cast.hpp>

namespace 
{

const struct sockaddr* sockaddr_cast(const struct sockaddr_in* pSockAddr) {
	return static_cast<const struct sockaddr*>
		(boost::implicit_cast<const void*>(pSockAddr));
}

struct sockaddr* sockaddr_cast(struct sockaddr_in* pSockAddr) {
	return static_cast<struct sockaddr*>
		(boost::implicit_cast<void*>(pSockAddr));
}

}

Socket::Socket(int nSockFd) : m_nSockFd(nSockFd) {
	assert(nSockFd >= 0);
}

Socket::~Socket() {
	if (m_nSockFd >= 0) {
		int nRet = ::close(m_nSockFd);
		assert(0 == nRet);
		m_nSockFd = 0;
	}
}

int Socket::GetFd() const {
	return m_nSockFd;
}

void Socket::Bind(const InetAddress& rAddr) {
	const struct sockaddr_in& sockAddr = rAddr.GetSockAddrInet();
	int nRet = ::bind(m_nSockFd, sockaddr_cast(&sockAddr), sizeof(sockAddr));
	if (nRet) {
		perror("Socket::BindAddress()");
		abort();
	}
}

void Socket::Listen() {
	int nRet = ::listen(m_nSockFd, SOMAXCONN);
	if (nRet) {
		perror("Socket::Listen()");
		abort();
	}
}

int Socket::Connet(const InetAddress& rAddr) {
	const struct sockaddr_in& sockAddr = rAddr.GetSockAddrInet();
	return ::connect(m_nSockFd, sockaddr_cast(&sockAddr), sizeof(sockAddr));
}

void Socket::ShutdownWrite() {
	if (::shutdown(m_nSockFd, SHUT_WR) < 0) {
		perror("Socket::ShutdownWrite");
	}
}

void Socket::SetTcpNoDelay(bool bOn) {
	int nOptVal = bOn ? 1 : 0;
	if (::setsockopt(m_nSockFd, IPPROTO_TCP, TCP_NODELAY
		, &nOptVal, static_cast<socklen_t>(sizeof(nOptVal)) < 0)) 
	{
		perror("Socket::SetTcpNoDelay()");
	}
}

void Socket::SetReuseAddr(bool bOn) {
	int nOptVal = bOn ? 1 : 0;
	if (::setsockopt(m_nSockFd, SOL_SOCKET, SO_REUSEADDR
		, &nOptVal, sizeof(nOptVal) < 0)) 
	{
		perror("Socket::SetReuseAddr()");
	}
}

Socket Socket::CreateTcp() {
	int nSockFd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	assert(nSockFd >= 0);
	return Socket(nSockFd);
}
