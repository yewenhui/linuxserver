#include "LogFile.h"

#include <assert.h>
#include <stdio.h>
#include <time.h>

class LogFile::File 
	: private boost::noncopyable
{
public:
	explicit File(const string& rStrFileName)
		// : m_pFP(::fopen(rStrFileName.data(), "ae"))
		: m_pFP(::fopen(rStrFileName.data(), "a"))
		, m_unWrittenBytes(0)
	{
		::setbuffer(m_pFP, m_Buffer, sizeof(m_Buffer));
	}

	~File() {
		::fclose(m_pFP);
	}

	void		Append(const char* pLogLine, const size_t unLength) {
		size_t n = write(pLogLine, unLength);
		size_t nRemain = unLength - n;
		while (nRemain > 0) {
			size_t x = write(pLogLine + n, nRemain);
			if (0 == x) {
				int nErr = ::ferror(m_pFP);
				if (0 != nErr) {
					char buffer[128];
					strerror_r(nErr, buffer, sizeof(buffer));	// strerror_l
					fprintf(stderr, "LogFile::File::append() failed %s\n", buffer);
				}
				break;
			}
			n += x;
			nRemain = unLength - n;
		}

		m_unWrittenBytes += unLength;
	}

	void		Flush() {
		::fflush(m_pFP);
	}

	size_t		GetWrittenBytes() const {
		return m_unWrittenBytes;
	}

private:
	size_t		write(const char* pLogLine, const size_t unLength) {
		return ::fwrite_unlocked(pLogLine, 1, unLength, m_pFP);
		// return ::fwrite_unlocked(pLogLine, unLength, 1, m_pFP);
		// return ::fwrite(pLogLine, unLength, 1, m_pFP);
	}

private:
	FILE*		m_pFP;
	char		m_Buffer[64 * 1024];
	size_t		m_unWrittenBytes;

};

LogFile::LogFile(const string& strBaseName, size_t unRollSize, bool bThreadSafe /*= false */, int nFlushInterval /*= 3*/) 
	: m_strBaseName(strBaseName)
	, m_unRollSize(unRollSize)
	, m_nFlushInterval(nFlushInterval)
	, m_nCnt(0)
	, m_pMutex(bThreadSafe ? new MutexLock() : NULL)
	, m_tStartOfPeriod(0)
	, m_tLastRoll(0)
	, m_tLaseFlush(0)
{
	assert(strBaseName.find("/") == string::npos);
	rollFile();
}

LogFile::~LogFile() {

}

void LogFile::Append(const char* pLogLine, int nLen) {
	if (NULL != m_pMutex) {
		MutexLockGuard lock(*m_pMutex);
		appendUnlocked(pLogLine, nLen);
	} else {
		appendUnlocked(pLogLine, nLen);
	}
}

void LogFile::Flush() {
	if (NULL != m_pMutex) {
		MutexLockGuard lock(*m_pMutex);
		m_pFile->Flush();
	} else {
		m_pFile->Flush();
	}
}

void LogFile::appendUnlocked(const char* pStrLogLine, int nLen) {
	m_pFile->Append(pStrLogLine, nLen);
	if (m_pFile->GetWrittenBytes() > m_unRollSize) {
		rollFile();
	} else {
		if (m_nCnt > CHECK_TIME_ROLL) {
			m_nCnt = 0;
			time_t tNow = ::time(NULL);
			time_t tThisPeriod = tNow / ROLL_PER_SECONDS * ROLL_PER_SECONDS;
			if (tThisPeriod != m_tStartOfPeriod) {
				rollFile();
			} else if (tNow - m_tLaseFlush > m_nFlushInterval) {
				m_tLaseFlush = tNow;
				m_pFile->Flush();
			}
		} else {
			++ m_nCnt;
		}
	}
}

void LogFile::rollFile() {
	time_t tNow = 0;
	string strFileName = getLogFileName(m_strBaseName, &tNow);
	time_t tStart = tNow / ROLL_PER_SECONDS * ROLL_PER_SECONDS;
	if (tNow > m_tLastRoll) {
		m_tLastRoll = tNow;
		m_tLaseFlush = tNow;
		m_tStartOfPeriod = tStart;
		m_pFile.reset(new File(strFileName));
	}
}

string LogFile::getLogFileName(const string& rStrBaseNmae, time_t* pNow) {
	if (NULL == pNow) {
		assert(false);
		return rStrBaseNmae;
	}
	string strFileName;
	strFileName.reserve(rStrBaseNmae.size() + 32);
	strFileName = rStrBaseNmae;

	char timeBuff[32];

	struct tm curTm;
	*pNow = time(NULL);
	gmtime_r(pNow, &curTm); // localtime_r
	strftime(timeBuff, sizeof(timeBuff), ".%Y%m%d-%H%M%S", &curTm);
	strFileName += timeBuff;

	char pidBuff[32];
	snprintf(pidBuff, sizeof(pidBuff), ".%d", ::getpid());
	strFileName += pidBuff;
	strFileName += ".log";

	return strFileName;
}
