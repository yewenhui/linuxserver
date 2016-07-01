#include "InetAddress.h"
#include "SocketOps.h"

#include <assert.h>
#include <string.h>
#include <strings.h>		// bzero
#include <stdio.h>

#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>

using namespace std;

InetAddress::InetAddress(uint16_t usPort, bool bLoopBackOnly) {
	::bzero(&m_SockAddr, sizeof(m_SockAddr));
	m_SockAddr.sin_family = AF_INET;
	m_SockAddr.sin_port = Sockets::HostToNetwork16(usPort);
	m_SockAddr.sin_addr.s_addr = Sockets::HostToNetwork32(bLoopBackOnly ? INADDR_LOOPBACK : INADDR_ANY);
}

InetAddress::InetAddress(const std::string& rStrIp, uint16_t usPort) {
	::bzero(&m_SockAddr, sizeof(m_SockAddr));
	Sockets::FromHostPort(rStrIp.c_str(), usPort, m_SockAddr);
}

InetAddress::InetAddress(const struct sockaddr_in& sockAddr) : m_SockAddr(sockAddr) {

}

string InetAddress::ToHostPort() const {
	char buf[INET_ADDRSTRLEN];
	Sockets::ToHostPort(buf, sizeof(buf), m_SockAddr);
	return buf;
}

string InetAddress::ToIp() const {
	char buf[32] = "";
	::inet_ntop(AF_INET, &m_SockAddr.sin_addr, buf, sizeof(buf));
	return buf;
}

string InetAddress::ToIpPort() const {
	char buf[32] = "";
	::inet_ntop(AF_INET, &m_SockAddr.sin_addr, buf, sizeof(buf));
	int nEnd = ::strlen(buf);
	uint16_t usPort = GetPortHostEndian();
	snprintf(buf + nEnd, sizeof(buf) - nEnd, ":%u", usPort);
	return buf;
}

bool InetAddress::ResolveSlow(const char* pStrHostName, InetAddress* pOutAddr) {
	if (NULL == pStrHostName || NULL == pOutAddr) {
		assert(false);
		return false;
	}
	vector<char> vBuf(2 * RESOLVE_BUF_SIZE);
	struct hostent hent;
	::bzero(&hent, sizeof(hent));
	struct hostent* pHent = NULL;
	int nHErroNo = 0;

	while (vBuf.size() <= 16 * RESOLVE_BUF_SIZE) {
		int nRet = gethostbyname_r(pStrHostName, &hent
			, vBuf.data(), vBuf.size()
			, &pHent, &nHErroNo);

		if (0 == nRet && NULL != pHent) {
			if (pHent->h_addrtype != AF_INET || pHent->h_length != sizeof(uint32_t)) {
				assert(false);
				return false;
			}
			pOutAddr->m_SockAddr.sin_addr = *reinterpret_cast<struct in_addr*>(pHent->h_addr);
			return true;
		} else if (ERANGE == nRet) {
			vBuf.resize(2 * vBuf.size());
		} else {
			if (nRet) {
				perror("InetAddress::ResolveSlow()");
			}
			return false;
		}
	}

	return true;

}

bool InetAddress::Resolve(const char* pStrHostName, InetAddress* pOutAddr) {
	if (NULL == pStrHostName || NULL == pOutAddr) {
		assert(false);
		return false;
	}

	char strBuf[RESOLVE_BUF_SIZE];
	struct hostent hent;
	::bzero(&hent, sizeof(hent));
	struct hostent* pHent = NULL;
	int nHErroNo = 0;

	int nRet = gethostbyname_r(pStrHostName, &hent
		, strBuf, sizeof(strBuf)
		, &pHent, &nHErroNo);

	if (0 == nRet && NULL != pHent) {
		if (pHent->h_addrtype != AF_INET || pHent->h_length != sizeof(uint32_t)) {
			assert(false);
			return false;
		}
		pOutAddr->m_SockAddr.sin_addr = *reinterpret_cast<struct in_addr*>(pHent->h_addr);
		return true;
	} else if (ERANGE == nRet) {
		return ResolveSlow(pStrHostName, pOutAddr);
	} else {
		if (nRet) {
			perror("InetAddress::ResolveSlow()");
		}
		return false;
	}

	return true;
}
