#ifndef _ACCEPTOR_H_
#define _ACCEPTOR_H_

#include "Socket.h"

#include <memory>

#include <boost/noncopyable.hpp>

class InetAddress;
class TcpStream;
typedef std::unique_ptr<TcpStream> TcpStreamPtr;

class Acceptor
	: private boost::noncopyable
{
public:
	explicit Acceptor(const InetAddress& rListenAddr);
	~Acceptor() {}

	TcpStreamPtr	Accept();

private:
	Socket			m_ListenSocket;

};

#endif // _ACCEPTOR_H_
