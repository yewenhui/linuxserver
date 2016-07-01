#include "LogStream.h"

#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

#include <algorithm>
#include <limits>

#include <boost/static_assert.hpp>
#include <boost/type_traits/is_arithmetic.hpp>

using namespace detail;

namespace detail
{

const char digits[] = "9876543210123456789";
const char* zero = digits + 9;
BOOST_STATIC_ASSERT(sizeof(digits) == 20);

const char digitsHex[] = "0123456789abcdef";
BOOST_STATIC_ASSERT(sizeof(digitsHex) == 17);

template<typename T>
size_t convert(char buf[], T value) {
	T i = value;
	char* p = buf;

	do {
		int nIndex = i % 10;
		i /= 10;
		*p++ = zero[nIndex];
	} while (0 != i);

	if (value < 0) {
		*p++ = '-';
	}
	*p = '\0';

	std::reverse(buf, p);
	return p - buf;
}

size_t convertHex(char buf[], uintptr_t value) {
	uintptr_t i = value;
	char* p = buf;

	do {
		int nIndex = i % 16;
		i /= 16;
		*p++ = digitsHex[nIndex];
	} while (i != 0);
	*p = '\0';

	std::reverse(buf, p);
	return p - buf;
}

};

void LogStream::staticCheck() {
	BOOST_STATIC_ASSERT(MAX_NUMERIC_SIZE - 10 > std::numeric_limits<double>::digits10);
	BOOST_STATIC_ASSERT(MAX_NUMERIC_SIZE - 10 > std::numeric_limits<long double>::digits10);
	BOOST_STATIC_ASSERT(MAX_NUMERIC_SIZE - 10 > std::numeric_limits<long>::digits10);
	BOOST_STATIC_ASSERT(MAX_NUMERIC_SIZE - 10 > std::numeric_limits<long long>::digits10);
}

template<typename T>
void LogStream::formatInteger(T v) {
	if (m_Buffer.GetAvail() > MAX_NUMERIC_SIZE) {
		size_t unLength = convert(m_Buffer.GetCurrent(), v);
		m_Buffer.Add(unLength);
	}
}

LogStream& LogStream::operator<<(bool bVal) {
	m_Buffer.Append(bVal ? "1" : "0", 1);
	return *this;
}

LogStream& LogStream::operator<<(short sVal) {
	*this << static_cast<int>(sVal);
	return *this;
}

LogStream& LogStream::operator<<(unsigned short usVal) {
	*this << static_cast<unsigned int>(usVal);
	return *this;
}

LogStream& LogStream::operator<<(int nVal) {
	formatInteger(nVal);
	return *this;
}

LogStream& LogStream::operator<<(unsigned int unVal) {
	formatInteger(unVal);
	return *this;
}

LogStream& LogStream::operator<<(long lVal) {
	formatInteger(lVal);
	return *this;
}

LogStream& LogStream::operator<<(unsigned long ulVal) {
	formatInteger(ulVal);
	return *this;
}

LogStream& LogStream::operator<<(long long llVal) {
	formatInteger(llVal);
	return *this;
}

LogStream& LogStream::operator<<(unsigned long long ullVal) {
	formatInteger(ullVal);
	return *this;
}

LogStream& LogStream::operator<<(float fVal) {
	*this << static_cast<double>(fVal);
	return *this;
}

LogStream& LogStream::operator<<(double dVal) {
	if (m_Buffer.GetAvail() >= MAX_NUMERIC_SIZE) {
		int nLength = snprintf(m_Buffer.GetCurrent(), MAX_NUMERIC_SIZE, "%.12g", dVal);
		m_Buffer.Add(nLength);
	}
	return *this;
}

LogStream& LogStream::operator<<(char cVal) {
	m_Buffer.Append(&cVal, 1);
	return *this;
}

LogStream& LogStream::operator<<(const void* pVoid) {
	uintptr_t v = reinterpret_cast<uintptr_t>(pVoid);
	if (m_Buffer.GetAvail() >= MAX_NUMERIC_SIZE) {
		char* buf = m_Buffer.GetCurrent();
		buf[0] = '0';
		buf[1] = 'x';
		size_t len = convertHex(buf + 2, v);
		m_Buffer.Add(len + 2);
	}
	return *this;
}

LogStream& LogStream::operator<<(const char* pCVal) {
	m_Buffer.Append(pCVal, strlen(pCVal));
	return *this;
}

LogStream& LogStream::operator<<(const T& val) {
	m_Buffer.Append(val.m_pStr, val.m_unLength);
	return *this;
}

LogStream& LogStream::operator<<(const string& rStr) {
	m_Buffer.Append(rStr.c_str(), rStr.size());
	return *this;
}

template<typename T>
Fmt::Fmt(const char* pStrFmt, T val) {
	BOOST_STATIC_ASSERT(boost::is_arithmetic<T>::value == true);
	m_nLength = snprintf(m_pStrBuffer, sizeof(m_pStrBuffer), pStrFmt, val);
	assert(static_cast<size_t>(m_nLength) < sizeof(m_pStrBuffer));
}

// Explicit instantiations
template Fmt::Fmt(const char* fmt, short);
template Fmt::Fmt(const char* fmt, unsigned short);
template Fmt::Fmt(const char* fmt, int);
template Fmt::Fmt(const char* fmt, unsigned int);
template Fmt::Fmt(const char* fmt, long);
template Fmt::Fmt(const char* fmt, unsigned long);
template Fmt::Fmt(const char* fmt, long long);
template Fmt::Fmt(const char* fmt, unsigned long long);
template Fmt::Fmt(const char* fmt, float);
template Fmt::Fmt(const char* fmt, double);
