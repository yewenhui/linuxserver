#include "../AsyncLoggingQueue.h"
#include "../AsyncLoggingDoubleBuffering.h"
#include "../Logging.h"
#include "../../datetime/TimeStamp.h"

#include <stdio.h>
#include <sys/resource.h>

const unsigned int	ROLL_SIZE = 500 * 1000 * 1000;
const int			BATCH = 1000;
const bool			LONG_LOG = true;

void* g_pAsyncLog = NULL;

template<typename ASYNC>
void asyncOutput(const char* pStrMsg, int nLength) {
	ASYNC* pLog = static_cast<ASYNC*>(g_pAsyncLog);
	pLog->Append(pStrMsg, nLength);
}

template<typename ASYNC>
void bench(ASYNC* pLog) {
	g_pAsyncLog = pLog;
	pLog->Start();
	Logger::SetOutputFunc(asyncOutput<ASYNC>);

	int nCnt = 0;
	
	string empty = " ";
	string longStr(3000, 'X');
	longStr += " ";

	for (int t = 0; t < 30; ++t) {
		TimeStamp start = TimeStamp::Now();
		for (int n = 0; n < BATCH; ++n) {
			LOG_INFO << "Hello 0123456789" << " abcdefghijklmnopqrstuvwxyz "
				<< (LONG_LOG ? longStr : empty) << nCnt;
			++nCnt;
		}
		TimeStamp end = TimeStamp::Now();
		printf("%f\n", TimeDifference(end, start) * 1000000 / BATCH);
		struct timespec ts = { 0, 500 * 1000 * 1000 };
		nanosleep(&ts, NULL);
	}
}

int main(int argc, char* argv[]) {
	{
		// set max virtual memory to 2GB.
		size_t kOneGB = 1024 * 1024 * 1024;
		rlimit rl = { 2.0 * kOneGB, 2.0 * kOneGB };
		setrlimit(RLIMIT_AS, &rl);
	}

	AsyncLoggingUnboundedQueue  log1("log1", ROLL_SIZE);
	AsyncLoggingBoundedQueue    log2("log2", ROLL_SIZE, 1024);
	AsyncLoggingUnboundedQueueL log3("log3", ROLL_SIZE);
	AsyncLoggingBoundedQueueL   log4("log4", ROLL_SIZE, 1024);
	AsyncLoggingDoubleBuffering log5("log5", ROLL_SIZE);

	int which = argc > 1 ? atoi(argv[1]) : 1;

	printf("pid = %d\n", getpid());

	switch (which) {
	case 1:
		bench(&log1);
		break;
	case 2:
		bench(&log2);
		break;
	case 3:
		bench(&log3);
		break;
	case 4:
		bench(&log4);
		break;
	case 5:
		bench(&log5);
		break;
	}

	return 0;
}
