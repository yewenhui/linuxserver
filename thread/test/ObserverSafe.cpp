#include "../Mutex.h"

#include <vector>
#include <stdio.h>

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

class Observable;

class Observer
	: public boost::enable_shared_from_this<Observer>
{
public:
	Observer() : m_pSubject(NULL) {}
	virtual ~Observer() {}
	
	virtual void	update() = 0;

	void			observe(Observable* pSubject);

protected:
	Observable*		m_pSubject;

};

class Observable
{
	typedef std::vector<boost::weak_ptr<Observer> > VecObserver;

public:
	Observable() {};
	~Observable() {};

	void			regObserver(boost::weak_ptr<Observer> pObserver) {
		m_vObservers.push_back(pObserver);
	}

	void			notifyObservers() {
		MutexLockGuard lock(m_Mutex);
		for (VecObserver::iterator itor = m_vObservers.begin(), end = m_vObservers.end()
			; end != itor
			; ++itor) 
		{
			boost::shared_ptr<Observer> pObser(itor->lock());
			if (NULL != pObser) {
				pObser->update();
			} else {
				printf("notifyObservers() erase\n");
				itor = m_vObservers.erase(itor);
			}
		}
	}

private:
	mutable MutexLock m_Mutex;
	VecObserver		m_vObservers;

};

void Observer::observe(Observable* pSubject) {
	if (NULL == pSubject) {
		return;
	}
	pSubject->regObserver(shared_from_this());
	m_pSubject = pSubject;
}

class Foo
	: public Observer
{
public:
	virtual void		update() {
		 printf("Foo::update() %p\n", this);
	}

};

int main()
{
	Observable subject;
	{
		boost::shared_ptr<Foo> pObser(new Foo());
		pObser->observe(&subject);
		subject.notifyObservers();
	}

	subject.notifyObservers();
	return 0;
}
