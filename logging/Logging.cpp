#include "Logging.h"

#include "../thread/Thread.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <sstream>

__thread char t_errnoBuf[512];
__thread char t_time[32];
__thread time_t t_lastSecond;

const char* StrError_tl(int nSavedErrno) {
	return strerror_r(nSavedErrno, t_errnoBuf, sizeof(t_errnoBuf));
}

Logger::LogLevel InitLogLevel() {
	if (::getenv("LOG_TRACE")) {
		return Logger::TRACE;
	}
	return Logger::DEBUG;
	// return Logger::TRACE;
}

Logger::LogLevel g_LogLevel = InitLogLevel();

const char* LogLevelName[Logger::LOG_LEVEL_CNT] = {
	"TRACE"			, 
	"DEBUG"			, 
	"INFO"			, 
	"WARNING"		, 
	"ERROR"			, 
	"FATAL"			, 
};

void DefaultOutput(const char* pStrMsg, int nLen) {
	/*size_t n = */fwrite(pStrMsg, nLen, 1, stdout);
	// 
}

void DefaultFlush() {
	fflush(stdout);
}

Logger::OutputFunc g_OutputFunc = DefaultOutput;
Logger::FlushFunc g_FlushFunc = DefaultFlush;

Logger::Logger(const char* pStrFile, int nLine) 
	: m_Impl(INFO, 0, pStrFile, nLine)
{

}

Logger::Logger(const char* pStrFile, int nLine, LogLevel eLevel) 
	: m_Impl(eLevel, 0, pStrFile, nLine)
{

}

Logger::Logger(const char* pStrFile, int nLine, LogLevel eLevel, const char* pStrFunc)
	: m_Impl(eLevel, 0, pStrFile, nLine)
{
	m_Impl.stream << pStrFunc << ' ';
}

Logger::Logger(const char* pStrFile, int nLine, bool bAbort)
	: m_Impl(bAbort ? FATAL : ERROR, errno, pStrFile, nLine)
{

}

Logger::~Logger() {
	m_Impl.Finish();
	const LogStream::Buffer& buf(Stream().GetBuffer());
	g_OutputFunc(buf.GetData(), buf.GetLength());
	if (FATAL == m_Impl.level) {
		g_FlushFunc();
		::abort();
	}
}

Logger::LogLevel Logger::GetLogLevel() {
	return g_LogLevel;
}

void Logger::SetLogLevel(LogLevel eLevel) {
	g_LogLevel = eLevel;
}

void Logger::SetOutputFunc(OutputFunc func) {
	g_OutputFunc = func;
}

void Logger::SetFlushFunc(FlushFunc func) {
	g_FlushFunc = func;
}

Logger::Impl::Impl(LogLevel eLevel, int nOldErrno, const char* pStrFile, int nLine)
	: timeStamp(TimeStamp::Now())
	, stream()
	, level(eLevel)
	, nLine(nLine)
	, pStrFullName(pStrFile)
	, pStrBaseName(NULL)
{
	const char* pStrPathSepPos = strrchr(pStrFullName, '/');
	pStrBaseName = (NULL != pStrPathSepPos) ? pStrPathSepPos + 1 : pStrFullName;

	FormatTime();
// 	Fmt tid("%6d", CurrentThread::GetTid());
// 	stream << tid.GetData();
	stream << "[" << LogLevelName[level] << "]\t";
	if (0 != nOldErrno) {
		stream << StrError_tl(nOldErrno) << " (errno=" << nOldErrno << ") ";
	}
}

void Logger::Impl::FormatTime() {
	int64_t microSecondsSinceEpoch = timeStamp.GetMicroSecondsSinceEpoch();
	time_t seconds = static_cast<time_t>(microSecondsSinceEpoch / TimeStamp::MICRO_SECOND_PER_SECOND);
	int microSeconds = static_cast<int>(microSecondsSinceEpoch % TimeStamp::MICRO_SECOND_PER_SECOND);
	if (seconds != t_lastSecond) {
		t_lastSecond = seconds;
		struct tm tm_time;
		::gmtime_r(&seconds, &tm_time);

		int len = snprintf(t_time, sizeof(t_time), "%4d%02d%02d %02d:%02d:%02d"
			, tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday
			, tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
		assert(len == 17);
	}

	Fmt us(".%06dZ ", microSeconds);
	assert(us.GetLength() == 9);
	stream << T(t_time, 17) << T(us.GetData(), 9);
}

void Logger::Impl::Finish() {
	stream << "\t" << pStrBaseName << ":" << nLine << '\n';
}
