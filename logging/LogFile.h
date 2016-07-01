#ifndef _LOG_FILE_H_ 
#define _LOG_FILE_H_

#include "../thread/Mutex.h"

#include <string>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>

using std::string;

class LogFile
	: private boost::noncopyable
{
	enum {
		CHECK_TIME_ROLL		= 1024,
		ROLL_PER_SECONDS	= 60 * 60 * 24,
	};

	class File;

public:
	LogFile(const string& strBaseName
		, size_t unRollSize
		, bool bThreadSafe = false
		, int nFlushInterval = 3);
	~LogFile();

	void			Append(const char* pLogLine, int nLen);
	void			Flush();

private:
	static string	getLogFileName(const string& rStrBaseNmae, time_t* pNow);
	
	void			appendUnlocked(const char* pStrLogLine, int nLen);
	void			rollFile();

private:
	const string	m_strBaseName;
	const unsigned int m_unRollSize;
	const int		m_nFlushInterval;

	int				m_nCnt;

	boost::scoped_ptr<MutexLock> m_pMutex;
	time_t			m_tStartOfPeriod;
	time_t			m_tLastRoll;
	time_t			m_tLaseFlush;
	boost::scoped_ptr<File> m_pFile;

};

#endif // _LOG_FILE_H_
