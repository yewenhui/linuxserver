#ifndef _INET_ADDRESS_H_
#define _INET_ADDRESS_H_

#include <string>
#include <vector>

#include <netinet/in.h>

using std::string;

class InetAddress
{
	enum {
		RESOLVE_BUF_SIZE		= 4096,		// RFC6891: EDNS payload 4096 bytes
	};

public:
	explicit InetAddress(uint16_t usPort, bool bLoopBackOnly = false);
	InetAddress(const struct sockaddr_in& sockAddr);

	string				ToIp() const;
	string				ToIpPort() const;

	const struct sockaddr_in& GetSockAddrInet() const { return m_SockAddr; }
	void				SetSockAddrInet(struct sockaddr_in& rSockAddr) { m_SockAddr = rSockAddr; }

	void				SetPort(uint16_t usPort) { m_SockAddr.sin_port = htons(usPort); }

	uint32_t			GetIpNetEndian() const { return m_SockAddr.sin_addr.s_addr; }
	uint16_t			GetPortNetEndian() const { return m_SockAddr.sin_port; }

	uint32_t			GetIpHostEndian() const { return ntohl(m_SockAddr.sin_addr.s_addr); }
	uint16_t			GetPortHostEndian() const { return ntohs(m_SockAddr.sin_port); }

	static bool			Resolve(const char* pStrHostName, InetAddress* pOutAddr);	

private:
	static bool			ResolveSlow(const char* pStrHostName, InetAddress* pOutAddr);

private:
	struct sockaddr_in	m_SockAddr;

};

#endif // _INET_ADDRESS_H_
