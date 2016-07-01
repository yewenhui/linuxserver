#include "../Logging.h"
#include "../LogFile.h"

#include <stdio.h>

long g_total;
FILE* g_file;
boost::scoped_ptr<LogFile> g_logFile;

void dummyOutput(const char* pStrMsg, int nLength) {
	g_total += nLength;
	if (NULL != g_file) {
		fwrite(pStrMsg, 1, nLength, g_file);
	} else if (g_logFile) {
		g_logFile->Append(pStrMsg, nLength);
	}
}

void bench() {
	Logger::SetOutputFunc(dummyOutput);
	TimeStamp start(TimeStamp::Now());

	g_total = 0;
	const int batch = 1000 * 1000;
	const bool bLongLog = false;
	string empty = " ";
	string longStr(3000, 'x');

	longStr += " ";
	for (int n = 0; n < batch; ++n) {
		LOG_INFO << "Hello 0123456789" << " abcdefghijklmnopqrstuvwxyz "
			<< (bLongLog ? longStr : empty) << n;
	}

	TimeStamp end(TimeStamp::Now());
	double dSeconds = TimeDifference(end, start);
	printf("%f seconds, %ld bytes, %.2f msg/s, %.2f MiB/s\n"
		, dSeconds, g_total, batch / dSeconds, g_total / dSeconds / 1024 / 1024);
}

int main() {
	// for ltrace and strace
	// getppid();

	LOG_TRACE << "trace";
	LOG_DEBUG << "debug";
	LOG_INFO << "Hello";
	LOG_WARNING << "Warning";
	LOG_ERROR << "Error";
	LOG_INFO << sizeof(Logger);
	LOG_INFO << sizeof(LogStream);
	LOG_INFO << sizeof(Fmt);
	LOG_INFO << sizeof(LogStream::Buffer);
	
	bench();

	char buffer[64 * 1024];

	g_file = fopen("/dev/null", "w");
	setbuffer(g_file, buffer, sizeof(buffer));
	bench();
	fclose(g_file);

	g_file = fopen("/tmp/log", "w");
	setbuffer(g_file, buffer, sizeof(buffer));
	bench();
	fclose(g_file);

	g_file = NULL;
	g_logFile.reset(new LogFile("test_log", 500 * 1000 * 1000));
	bench();

	g_logFile.reset(new LogFile("test_log_fmt", 500 * 1000 * 1000, true));
	bench();
	g_logFile.reset();

}

