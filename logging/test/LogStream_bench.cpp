#include "../../datetime/TimeStamp.h"
#include "../LogStream.h"

#include <sstream>
#include <stdio.h>

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

const unsigned int NUMBER = 1000000;

template<typename T>
void BenchPrintf(const char* pStrFmt) {
	char buf[32];
	TimeStamp start(TimeStamp::Now());
	for (size_t n = 0; n < NUMBER; ++n) {
		snprintf(buf, sizeof(buf), pStrFmt, (T)(n));
	}
	TimeStamp end(TimeStamp::Now());

	printf("BenchPrintf %f\n", TimeDifference(end, start));
}

template<typename T>
void BenchStringStream() {
	TimeStamp start(TimeStamp::Now());
	std::ostringstream os;
	for (size_t n = 0; n < NUMBER; ++n) {
		os << (T)(n);
		os.seekp(0, std::ios_base::beg);
	}
	TimeStamp end(TimeStamp::Now());
	printf("BenchStringStream %f\n", TimeDifference(end, start));
}

template<typename T>
void BenchLogStream() {
	TimeStamp start(TimeStamp::Now());
	LogStream os;
	for (size_t n = 0; n < NUMBER; ++n) {
		os << (T)(n);
		os.ResetBuffer();
	}
	TimeStamp end(TimeStamp::Now());

	printf("BenchLogStream %f\n", TimeDifference(end, start));
}

int main() {
	BenchPrintf<int>("%d");

	puts("int");
	BenchPrintf<int>("%d");
	BenchStringStream<int>();
	BenchLogStream<int>();

	puts("double");
	BenchPrintf<double>("%.12g");
	BenchStringStream<double>();
	BenchLogStream<double>();

	puts("int64_t");
	BenchPrintf<int64_t>("%" PRId64);
	BenchStringStream<int64_t>();
	BenchLogStream<int64_t>();

	puts("void*");
	BenchPrintf<void*>("%p");
	BenchStringStream<void*>();
	BenchLogStream<void*>();
}

