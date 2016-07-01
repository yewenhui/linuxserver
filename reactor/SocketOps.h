#ifndef _SOCKET_OPTS_H_ 
#define _SOCKET_OPTS_H_

#include <arpa/inet.h>
#include <endian.h>

/*
	网络的字节流是大端
*/

namespace Sockets
{
	inline uint64_t HostToNetwork64(uint64_t hostVal) {
		return htobe64(hostVal);
	}

	inline uint32_t HostToNetwork32(uint32_t hostVal) {
		return htonl(hostVal);
	}

	inline uint16_t HostToNetwork16(uint16_t hostVal) {
		return htons(hostVal);
	}

	inline uint64_t NetworkToHost64(uint64_t netVal) {
		return be64toh(netVal);
	}

	inline uint32_t NetworkToHost32(uint32_t netVal) {
		return ntohl(netVal);
	}

	inline uint16_t NetworkToHost16(uint16_t netVal) {
		return ntohs(netVal);
	}

	int				CreateNonBlockingOrDie();

	int				Connect(int nSockFd, const struct sockaddr_in& rAddr); 
	void			BindOrDie(int nSockFd, const struct sockaddr_in& rAddr);
	void			ListenOrDie(int nSockFd);
	int				Accept(int nSockFd, struct sockaddr_in& rAddr);
	void			Close(int nSockFd);
	void			ShutDownWrite(int nSockFd);

	void			ToHostPort(char* pStrBuf, size_t unSize, const struct sockaddr_in& rAddr);
	void			FromHostPort(const char* pStrIp, uint16_t usPort, struct sockaddr_in& rAddr);

	struct sockaddr_in GetLocalAddr(int nSockFd);
	struct sockaddr_in GetPeerAddr(int nSockFd);

	int				GetSocketError(int nSockFd);
	bool			IsSelfConnect(int nSockFd);
}

#endif // _SOCKET_OPTS_H_
