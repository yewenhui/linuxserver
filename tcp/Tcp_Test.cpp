#include "Acceptor.h"
#include "InetAddress.h"
#include "TcpStream.h"

#include <iostream>
#include <sys/time.h>

#include <boost/program_options.hpp>

using namespace std;

struct Options
{
	uint16_t port;
	int length;
	int number;
	bool transmit, receive, nodelay;
	std::string host;
	Options()
		: port(0), length(0), number(0)
		, transmit(false), receive(false), nodelay(false)
	{
	}
};

struct SessionMessage
{
	int32_t number;
	int32_t length;
} __attribute__ ((__packed__));

struct PayloadMessage
{
	int32_t length;
	char data[0];
};

double now() {
	struct timeval tv = { 0, 0 };
	gettimeofday(&tv, NULL);
	return tv.tv_sec + tv.tv_usec / 1000000.0;
}

bool parseCommandLine(int argc, char* argv[], Options* pOpt) {
	if (NULL == pOpt) {
		assert(false);
		return false;
	}
	
	namespace BOOST_OPTION = boost::program_options;

	BOOST_OPTION::options_description desc("Allowed Options");
	desc.add_options()
		("help, h",		"Help")
		("port, p",		BOOST_OPTION::value<uint16_t>(&pOpt->port)->default_value(5001),	"TCP port")
		("length, l",	BOOST_OPTION::value<int>(&pOpt->length)->default_value(65536),		"Buffer length")
		("number, n",	BOOST_OPTION::value<int>(&pOpt->number)->default_value(8192),		"Number of buffers")
		("trans, t",	BOOST_OPTION::value<std::string>(&pOpt->host), "Transmit")
		("recv, r",		"Receive")
		("nodelay, D",	"set TCP_NODELAY")	
	;

	BOOST_OPTION::variables_map mpVariable;
	BOOST_OPTION::store(BOOST_OPTION::parse_command_line(argc, argv, desc), mpVariable);
	BOOST_OPTION::notify(mpVariable);

	pOpt->transmit = mpVariable.count("trans");
	pOpt->receive = mpVariable.count("recv");
	pOpt->nodelay = mpVariable.count("nodelay");

	if (mpVariable.count("help")) {
		std::cout << desc << std::endl;
		return false;
	}

	if (pOpt->transmit == pOpt->receive) {
		printf("either -t or -r must be specified.\n");
		return false;
	}

	printf("port = %d\n", pOpt->port);
	if (pOpt->transmit) {
		printf("buffer length = %d\n", pOpt->length);
		printf("number of buffers = %d\n", pOpt->number);
	} else {
		printf("accepting...\n");
	}
	return true;
}

void transmit(const Options& rOption) {
	InetAddress addr(rOption.port);
	if (!InetAddress::Resolve(rOption.host.c_str(), &addr)) {
		printf("unable to resolve %s\n", rOption.host.c_str());
		return;
	}

	printf("conneting to %s\n", addr.ToIpPort().c_str());
	TcpStreamPtr pTcpStream(TcpStream::Connect(addr));
	if (NULL == pTcpStream) {
		printf("unable to connect %s\n", addr.ToIpPort().c_str());
		return;
	}

	if (rOption.nodelay) {
		pTcpStream->SetNoDelay(true);
	}
	printf("connected\n");

	double dStart = now();
	struct SessionMessage sessionMsg = { 0, 0 };
	sessionMsg.number = htonl(rOption.number);
	sessionMsg.length = htonl(rOption.length);

	if (pTcpStream->SendAll(&sessionMsg, sizeof(sessionMsg)) != sizeof(sessionMsg)) {
		printf("write SessionMessage error");
		return;
	}

	const int nTotalLength = sizeof(int32_t) + rOption.length;
	PayloadMessage* pPayLoadMsg = static_cast<PayloadMessage*>(::malloc(nTotalLength));
	std::unique_ptr<PayloadMessage, void(*)(void*)> pFreeIt(pPayLoadMsg, ::free);
	assert(pPayLoadMsg);

	pPayLoadMsg->length = htonl((rOption.length));
	for (int n = 0; n < rOption.length; ++n) {
		pPayLoadMsg->data[n] = "0123456789ABCDEF"[n % 16];
	}

	double dTotalMb = 1.0 * rOption.length * rOption.number / 1024 / 1024;
	printf("%.3f MiB in total\n", dTotalMb);

	for (int i = 0; i < rOption.number; ++i) {
		int nWriteNum = pTcpStream->SendAll(pPayLoadMsg, nTotalLength);
		assert(nWriteNum == nTotalLength);

		int nAck = 0;
		int nReadNum = pTcpStream->ReadAll(&nAck, sizeof(nAck));
		assert(nReadNum == sizeof(nAck));
		nAck = ntohl(nAck);
		assert(nAck == rOption.length);
	}
	
	double dElapsed = now() - dStart;
	printf("%.3f seconds\n%.3f MiB/s\n", dElapsed, dTotalMb / dElapsed);
}

void receive(const Options& rOption) {
	Acceptor acceptor(InetAddress(rOption.port));
	TcpStreamPtr pTcpStream(acceptor.Accept());
	if (NULL != pTcpStream) {
		return;
	}

	struct SessionMessage sessionMessage = { 0, 0 };
	if (pTcpStream->ReadAll(&sessionMessage, sizeof(sessionMessage)) != sizeof(sessionMessage)) {
		printf("read SessionMessage");
		return;
	}

	sessionMessage.number = ntohl(sessionMessage.number);
	sessionMessage.length = ntohl(sessionMessage.length);
	printf("receive buffer length = %d\nreceive number of buffers = %d\n"
			, sessionMessage.length
			, sessionMessage.number);
	double nTotalMb = 1.0 * sessionMessage.number * sessionMessage.length / 1024 / 1024;
	printf("%.3f MiB in total\n", nTotalMb);

	const int nTotalLength = sizeof(int32_t) + rOption.length;
	PayloadMessage* pPayLoadMsg = static_cast<PayloadMessage*>(::malloc(nTotalLength));
	std::unique_ptr<PayloadMessage, void(*)(void*)> pFreeIt(pPayLoadMsg, ::free);
	assert(pPayLoadMsg);

	double dStart = now();

	for (int n = 0; n < sessionMessage.number; ++n) {
		pPayLoadMsg->length = 0;
		if (pTcpStream->ReadAll(&pPayLoadMsg->length, sizeof(pPayLoadMsg->length))
			!= sizeof(pPayLoadMsg->length))
		{
			printf("read length");
			return;
		}

		pPayLoadMsg->length = ntohl(pPayLoadMsg->length);
		assert(pPayLoadMsg->length == sessionMessage.length);

		if (pTcpStream->ReadAll(pPayLoadMsg->data, pPayLoadMsg->length)
			!= pPayLoadMsg->length)
		{
			printf("read payload data");
			return;
		}

		int nAck = htonl(pPayLoadMsg->length);
		if (pTcpStream->SendAll(&nAck, sizeof(nAck)) != sizeof(nAck)) {
			printf("write ack");
			return;
		}
	}

	double dElapsed = now() - dStart;
	printf("%.3f seconds\n%.3f MiB/s\n", dElapsed, nTotalLength / dElapsed);
}

int main(int argc, char* argv[]) {
	Options options;
	if (parseCommandLine(argc, argv, &options)) {
		if (options.transmit) {
			transmit(options);
		} else if (options.receive)  {
			receive(options);
		} else {
			assert(false);
		}
	}
}
