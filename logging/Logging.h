#ifndef _LOGGING_H_ 
#define _LOGGING_H_

#include "LogStream.h"
#include "../datetime/TimeStamp.h"

#include <boost/scoped_ptr.hpp>
#include <boost/function.hpp>

class Logger
{
public:
	enum LogLevel {
		TRACE			= 0,
		DEBUG			,
		INFO			,
		WARNING			,
		ERROR			,
		FATAL			,
		LOG_LEVEL_CNT	,
	};

	typedef boost::function<void(const char*, int)> OutputFunc;
	typedef boost::function<void()> FlushFunc;

public:
	Logger(const char* pStrFile, int nLine);
	Logger(const char* pStrFile, int nLine, LogLevel eLevel);
	Logger(const char* pStrFile, int nLine, LogLevel eLevel, const char* pStrFunc);
	Logger(const char* pStrFile, int nLine, bool bAbort);
	~Logger();

	LogStream&		Stream() { return m_Impl.stream; }

	static LogLevel GetLogLevel();
	static void		SetLogLevel(LogLevel eLevel);

	static void		SetOutputFunc(OutputFunc func);
	static void		SetFlushFunc(FlushFunc func);

private:
	class Impl
	{
	public:
		typedef Logger::LogLevel LogLevel;

		Impl(LogLevel eLevel, int nOldErrno, const char* pStrFile, int nLine);
		
		void		FormatTime();
		void		Finish();

		TimeStamp	timeStamp;
		LogStream	stream;
		LogLevel	level;
		int			nLine;

		const char* pStrFullName;
		const char* pStrBaseName;
	};

	Impl		m_Impl;

};

#define LOG_TRACE if (Logger::GetLogLevel() <= Logger::TRACE)	Logger(__FILE__, __LINE__, Logger::TRACE, __func__).Stream()
#define LOG_DEBUG if (Logger::GetLogLevel() <= Logger::DEBUG)	Logger(__FILE__, __LINE__, Logger::DEBUG, __func__).Stream()
#define LOG_INFO if (Logger::GetLogLevel() <= Logger::INFO)		Logger(__FILE__, __LINE__).Stream()
#define LOG_WARNING												Logger(__FILE__, __LINE__, Logger::WARNING).Stream()
#define LOG_ERROR												Logger(__FILE__, __LINE__, Logger::ERROR).Stream()
#define LOG_FATAL												Logger(__FILE__, __LINE__, Logger::FATAL).Stream()
#define LOG_SYSERR												Logger(__FILE__, __LINE__, false).Stream()
#define LOG_SYSFATAL											Logger(__FILE__, __LINE__, true).Stream()

const char* StrError_tl(int nSavedErrno);

#define CHECK_NOTNULL(val) \
			CheckNotNull(__FILE__, __LINE__, "'" #val "' Must be non NULL", (val))

template <typename T>
T* CheckNotNull(const char* pStrFile, int nLine, const char* pStrName, T* pT) {
	if (NULL == pT) {
		Logger(pStrFile, nLine, Logger::FATAL).Stream() << pStrName;
	}
	return pT;
}

// 
template<typename To, typename From>
inline To implicit_cast(From const &f) {
	return f;
}

#endif // _LOGGING_H_
