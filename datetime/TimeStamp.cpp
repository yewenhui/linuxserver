#include "TimeStamp.h"

#include <stdio.h>
#include <sys/time.h>

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include <boost/static_assert.hpp>

BOOST_STATIC_ASSERT(sizeof(TimeStamp) == sizeof(int64_t));



TimeStamp::TimeStamp() : m_MicroSecondsSinceEpoch(0) {

}

TimeStamp::TimeStamp(int64_t microSecondsSinceEpoch)
	: m_MicroSecondsSinceEpoch(microSecondsSinceEpoch) 
{

}

std::string TimeStamp::ToString() const {
	char buf[32] = { 0 };
	int64_t seconds = m_MicroSecondsSinceEpoch / MICRO_SECOND_PER_SECOND;
	int64_t microSeconds = m_MicroSecondsSinceEpoch % MICRO_SECOND_PER_SECOND;
	snprintf(buf, sizeof(buf), "%" PRId64 ".%06" PRId64 "", seconds, microSeconds);
	return buf;
}

std::string TimeStamp::ToFormattedString() const {
	char buf[32] = { 0 };
	int64_t seconds = m_MicroSecondsSinceEpoch / MICRO_SECOND_PER_SECOND;
	int64_t microSeconds = m_MicroSecondsSinceEpoch % MICRO_SECOND_PER_SECOND;
	struct tm tm_time;
	gmtime_r(&seconds, &tm_time);

	snprintf(buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d.%06d"
		, tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday
		, tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec
		, (int)microSeconds);
	return buf;
}

TimeStamp TimeStamp::Now() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	int64_t seconds = tv.tv_sec;
	return TimeStamp(seconds * MICRO_SECOND_PER_SECOND + tv.tv_usec);
}

TimeStamp TimeStamp::Invalid() {
	return TimeStamp();
}
