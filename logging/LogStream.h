#ifndef _LOG_STREAM_H_
#define _LOG_STREAM_H_

#include <assert.h>
#include <string.h>
#include <string>

#include <boost/noncopyable.hpp>

using std::string;

namespace detail 
{

const int SAMLL_BUFFER = 4000;
const int LARGE_BUFFER = 4000 * 1000;

template<int SIZE>
class FixedBuffer
	: private boost::noncopyable
{
public:
	FixedBuffer()
		: m_pCur(m_Data)
	{
	}

	~FixedBuffer() {
	}

	void		Append(const char* pStrBuf, int nLength) {
		if (GetAvail() > nLength) {
			memcpy(m_pCur, pStrBuf, nLength);
			m_pCur += nLength;
		}
	}

	const char*	GetData() const { return m_Data; }
	int			GetLength() const { return m_pCur - m_Data; }

	char*		GetCurrent() const { return m_pCur; }
	int			GetAvail() const { return static_cast<int>(end() - m_pCur); }

	void		Add(size_t unLength) { m_pCur += unLength; }
	void		Reset() { m_pCur = m_Data; }
	void		BZero() { ::bzero(m_Data, sizeof(m_Data)); }

	string		AsString() const { return string(m_Data, GetLength()); }
	const char*	DebugString() { *m_pCur = '\0'; return m_Data; }

private:
	const char*	end() const { return m_Data + sizeof(m_Data); }

private:
	char		m_Data[SIZE];
	char*		m_pCur;
	// CookieFunc	m_CookieFunc;
	void		(*m_CookieFunc)();
};

}


// helper类，为了在编译时间知道字符串长度
class T
{
public:
	T(const char* pStr, int nLength)
		: m_pStr(pStr), m_unLength(nLength)
	{
		assert(strlen(pStr) == m_unLength);
	}

	const char*	m_pStr;
	const size_t m_unLength;

};

class LogStream
	: private boost::noncopyable
{
	enum {
		MAX_NUMERIC_SIZE = 32,
	};

public:
	typedef detail::FixedBuffer<detail::SAMLL_BUFFER> Buffer;
	
	LogStream&	operator<<(bool bVal);
	LogStream&	operator<<(short sVal);
	LogStream&	operator<<(unsigned short usVal);
	LogStream&	operator<<(int nVal);
	LogStream&	operator<<(unsigned int unVal);
	LogStream&	operator<<(long lVal);
	LogStream&	operator<<(unsigned long ulVal);
	LogStream&	operator<<(long long llVal);
	LogStream&	operator<<(unsigned long long ullVal);
	LogStream&	operator<<(float fVal);
	LogStream&	operator<<(double dVal);
	LogStream&	operator<<(char cVal);

	LogStream&	operator<<(const void* pVoid);
	LogStream&	operator<<(const char* pCVal);
	
	LogStream&	operator<<(const T& val);
	LogStream&	operator<<(const string& rStr);

	void		Append(const char* pStrData, int nLength) { m_Buffer.Append(pStrData, nLength); }
	const Buffer& GetBuffer() const { return m_Buffer; }
	void		ResetBuffer() { m_Buffer.Reset(); }

private:
	void		staticCheck();
	template<typename T>
	void		formatInteger(T v);	

private:
	Buffer		m_Buffer;

};

class Fmt
{
public:
	template<typename T>
	Fmt(const char* pStrFmt, T val);

	const char*		GetData() const { return m_pStrBuffer; }
	int				GetLength() const { return m_nLength; }

private:
	char	m_pStrBuffer[32];
	int		m_nLength;

};

inline LogStream& operator<<(LogStream& stream, const Fmt& fmt) {
	stream.Append(fmt.GetData(), fmt.GetLength());
	return stream;
}

#endif // _LOG_STREAM_H_
