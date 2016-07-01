#ifndef _SINGLETON_H_
#define	_SINGLETON_H_

#include <boost/noncopyable.hpp>
#include <pthread.h>
#include <stdlib.h> // atexit

template<typename T>
class Singleton
	: private boost::noncopyable
{
public:
	static T&		Instance() {
		pthread_once(&m_POnce, &Singleton::init);
		return *m_pValue;
	}

private:
	Singleton() {}
	~Singleton() {}

	static void		init() {
		m_pValue = new T();
		::atexit(destroy);
	}

	static void		destroy() {
		if (NULL != m_pValue) {
			delete m_pValue;
			m_pValue = NULL;
		}
	}

private:
	static pthread_once_t	m_POnce;
	static T*				m_pValue;

};

template<typename T>
pthread_once_t Singleton<T>::m_POnce = PTHREAD_ONCE_INIT;

template<typename T>
T* Singleton<T>::m_pValue = NULL;

#endif	// _SINGLETON_H_
