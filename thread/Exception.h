#ifndef _EXCEPTION_H_
#define _EXCEPTION_H_

#include <exception>
#include <string>

class Exception
	: public std::exception
{
	enum {
		eBackTraceMaxLength		= 100,
	};

public:
	explicit Exception(const char* pStrWhat);
	virtual ~Exception() throw();

	virtual const char* what() const throw();
	const char*			StackTrace() const throw();

private:
	std::string			m_StrMsg;
	std::string			m_StrStack;

};

#endif // _EXCEPTION_H_
