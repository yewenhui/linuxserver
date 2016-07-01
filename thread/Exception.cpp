#include "Exception.h"

#include <execinfo.h>
#include <stdlib.h>		// free

Exception::Exception(const char* pStrWhat)
	: m_StrMsg(pStrWhat)
{
	void* pBuffer[eBackTraceMaxLength];
	int nPtrs = ::backtrace(pBuffer, eBackTraceMaxLength);
	char** ppStrBackTrace = backtrace_symbols(pBuffer, nPtrs);
	if (ppStrBackTrace) {
		for (int n = 0; n < nPtrs; ++n) {
			m_StrStack.append(ppStrBackTrace[n]);
			m_StrStack.push_back('\n');
		}
		// must free
		free(ppStrBackTrace);
	}
}

Exception::~Exception() throw() {

}

const char* Exception::what() const throw() {
	return m_StrMsg.c_str();
}

const char* Exception::StackTrace() const throw() {
	return m_StrStack.c_str();
}
