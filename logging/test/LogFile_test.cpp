#include "../LogFile.h"
#include "../Logging.h"

#include <boost/bind.hpp>

boost::scoped_ptr<LogFile> g_logFile;

void outputFunc(const char* msg, int len) {
	g_logFile->Append(msg, len);
}

void flushFunc() {
	g_logFile->Flush();
}

int main(int argc, char* argv[]) {
	char name[256];
	strncpy(name, argv[0], 256);
	g_logFile.reset(new LogFile(::basename(name), 256 * 1024));
 	Logger::SetOutputFunc(boost::bind(outputFunc, _1, _2));
 	Logger::SetFlushFunc(boost::bind(flushFunc));

	string line = "1234567890 abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ";

	for (int i = 0; i < 10000; ++i) {
		LOG_INFO << line;
		usleep(1000);
	}
} 
