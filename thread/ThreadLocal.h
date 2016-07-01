#ifndef _THREAD_LOCAL_H_
#define _THREAD_LOCAL_H_

#include <pthread.h>

#include <boost/noncopyable.hpp>

template<typename T>
class ThreadLocal
	: private boost::noncopyable
{
public:
	ThreadLocal() {
		pthread_key_create(&m_PKey, &ThreadLocal::destructor);
	}

	~ThreadLocal() {
		pthread_key_delete(m_PKey);
	}

	T&				Value() {
		T* perThreadValue = static_cast<T*>(pthread_getspecific(m_PKey));
		if (NULL == perThreadValue) {
			T* newObj = new T();
			pthread_setspecific(m_PKey, newObj);
			perThreadValue = newObj;
		}
		return *perThreadValue;
	}

private:
	static void		destructor(void* x) {
		T* obj = static_cast<T*>(x);
		delete obj;
	}

private:
	pthread_key_t	m_PKey;

};

#endif // _THREAD_LOCAL_H_