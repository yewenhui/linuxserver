#ifndef _BUFFER_H_
#define _BUFFER_H_

#include "SocketOps.h"

#include <algorithm>
#include <string>
#include <vector>

#include <assert.h>
#include <string.h>

/*
	+ ------------ + ------------ + ------------ + 
	|			   |			  |				 |		
	|	preapend   |	read	  |		write	 |
	|			   |			  |				 |
	0	  <=	readidx	  <=    writeidx   <=	size
*/
class Buffer
{
	enum {
		CHEAP_PREPEND		= 8,
		INITIAL_SIZE		= 1024,
	};

public:
	Buffer() 
		: m_vBuffer(CHEAP_PREPEND + INITIAL_SIZE)
		, m_unReaderIndex(CHEAP_PREPEND)
		, m_unWriterIndex(CHEAP_PREPEND)
	{
		assert(0 == ReadableBytes());
		assert(INITIAL_SIZE == WriteableBytes());
		assert(CHEAP_PREPEND == PrependableBytes());
	}
	// ~Buffer();

	void			Swap(Buffer& rBuffer) {
		m_vBuffer.swap(rBuffer.m_vBuffer);
		std::swap(m_unReaderIndex, rBuffer.m_unReaderIndex);
		std::swap(m_unWriterIndex, rBuffer.m_unWriterIndex);
	}
	
	size_t			PrependableBytes() const {
		return m_unReaderIndex;
	}
	size_t			ReadableBytes() const {
		return m_unWriterIndex - m_unReaderIndex;
	}
	size_t			WriteableBytes() const {
		return m_vBuffer.size() - m_unWriterIndex;
	}
	const char*		Peek() const {
		return begin() + m_unReaderIndex;
	}

	void			Retrieve(size_t unLength) {
		assert(unLength <= ReadableBytes());
		m_unReaderIndex += unLength;
	}
	void			RetrieveUntil(const char* pStrEnd) {
		assert(Peek() <= pStrEnd);
		assert(pStrEnd <= BeginWrite());
		Retrieve(pStrEnd - Peek());
	}
	void			RetrieveAll() {
		m_unReaderIndex = CHEAP_PREPEND;
		m_unWriterIndex = CHEAP_PREPEND;
	}
	std::string		RetrieveAsString() {
		std::string strRet(Peek(), ReadableBytes());
		RetrieveAll();
		return strRet;
	}

	void			RetrieveInt8() {
		Retrieve(sizeof(int8_t));
	}
	void			RetrieveInt16() {
		Retrieve(sizeof(int16_t));
	}
	void			RetrieveInt32() {
		Retrieve(sizeof(int32_t));
	}

	void			Append(const std::string& rStr) {
		Append(rStr.data(), rStr.length());
	}
	void			Append(const char* pStrData, size_t unLength) {
		EnsureWritableBytes(unLength);
		std::copy(pStrData, pStrData + unLength, BeginWrite());
		HasWritted(unLength);
	}
	void			Append(const void* pData, size_t unLength) {
		Append(static_cast<const char*>(pData), unLength);
	}

	void			AppendInt8(int8_t ucVal) {
		Append(&ucVal, sizeof(ucVal));
	}
	void			AppendInt16(int16_t usVal) {
		int16_t usAppendVal = Sockets::HostToNetwork16(usVal);
		Append(&usAppendVal, sizeof(usAppendVal));
	}
	void			AppendInt32(int32_t nVal) {
		int32_t nAppendVal = Sockets::HostToNetwork32(nVal);
		Append(&nAppendVal, sizeof(nAppendVal));
	}

	int8_t			ReadInt8() {
		int8_t result = PeekInt8();
		RetrieveInt8();
		return result;
	}
	int16_t			ReadInt16() {
		int16_t result = PeekInt16();
		RetrieveInt16();
		return result;
	}
	int32_t			ReadInt32() {
		int8_t result = PeekInt32();
		RetrieveInt32();
		return result;
	}

	int8_t			PeekInt8() {
		assert(ReadableBytes() >= sizeof(int8_t));
		int8_t ucRet = *Peek();
		return ucRet;
	}
	int16_t			PeekInt16() {
		assert(ReadableBytes() >= sizeof(int16_t));
		int16_t usRet = 0;
		::memcpy(&usRet, Peek(), sizeof(usRet));
		return Sockets::NetworkToHost16(usRet);
	}
	int32_t			PeekInt32() {
		assert(ReadableBytes() >= sizeof(int32_t));
		int32_t unRet = 0;
		::memcpy(&unRet, Peek(), sizeof(unRet));
		return Sockets::NetworkToHost32(unRet);
	}		

	void			EnsureWritableBytes(size_t unLength) {
		if (WriteableBytes() < unLength) {
			makeSpace(unLength);
		}
		assert(WriteableBytes() >= unLength);
	}

	char*			BeginWrite() {
		return begin() + m_unWriterIndex;
	}
	const char*		BeginWrite() const {
		return begin() + m_unWriterIndex;
	}

	void			HasWritted(size_t unLength) {
		m_unWriterIndex += unLength;
	}

	void			Prepend(const void* pData, size_t unLength) {
		assert(unLength <= PrependableBytes());
		m_unReaderIndex -= unLength;
		const char* pStrData = static_cast<const char*>(pData);
		std::copy(pStrData, pStrData + unLength, begin() + m_unReaderIndex);
	}

	void			Shrink(size_t unReserve) {
		std::vector<char> buf(CHEAP_PREPEND + ReadableBytes() + unReserve);
		std::copy(Peek(), Peek() + ReadableBytes(), buf.begin() + CHEAP_PREPEND);
		buf.swap(m_vBuffer);
	}

	ssize_t			ReadFd(int nFd, int* pSavedErrno);

private:
	char*			begin() {
		return &*m_vBuffer.begin();
	}
	const char*		begin() const {
		return &*m_vBuffer.begin();
	}
	void			makeSpace(size_t unLength) {
		if (WriteableBytes() + PrependableBytes() < unLength + CHEAP_PREPEND) {
			m_vBuffer.resize(m_unWriterIndex + unLength);
		} else {
			assert(CHEAP_PREPEND < m_unReaderIndex);
			size_t unReadable = ReadableBytes();
			std::copy(begin() + m_unReaderIndex, begin() + m_unWriterIndex, begin() + CHEAP_PREPEND);
			m_unReaderIndex = CHEAP_PREPEND;
			m_unWriterIndex = m_unReaderIndex + unReadable;
			assert(unReadable == ReadableBytes());
		}
	}

private:
	std::vector<char> m_vBuffer;
	size_t			m_unReaderIndex;
	size_t			m_unWriterIndex;

};

#endif // _BUFFER_H_
