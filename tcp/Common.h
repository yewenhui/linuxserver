#ifndef _COMMON_H_
#define _COMMON_H_

class noncopyable
{
protected:
	noncopyable() {}

private:
	noncopyable(const noncopyable& ) = delete;
	void operator=(const noncopyable&) = delete;

};

#endif // _COMMON_H_