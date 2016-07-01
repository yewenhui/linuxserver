#include <algorithm>
#include <vector>
#include <stdio.h>

class Observable;

class Observer
{
public:
	Observer() : m_pSubject(NULL) {}
	virtual	~Observer();

	virtual	void	update() = 0;

	void			observe(Observable* pObservable);

protected:
	Observable*		m_pSubject;

};

class Observable
{
public:
	Observable() {};
	~Observable() {};

	void			registerObser(Observer* pObserver) {
		if (NULL == pObserver) {
			return;
		}
		m_vObserver.push_back(pObserver);
	}

	void			UnRegisterObser(Observer* pObserver) {
		if (NULL == pObserver) {
			return;
		}
		std::vector<Observer*>::iterator itor = std::find(m_vObserver.begin(), m_vObserver.end(), pObserver);
		if (m_vObserver.end() != itor) {
			std::swap(*itor, m_vObserver.back());
			m_vObserver.pop_back();
		}
	}

	void			notifyObservers() {
		for (size_t n = 0; n < m_vObserver.size(); ++ n) {
			Observer* pObserver = m_vObserver[n];
			if (NULL != pObserver) {
				pObserver->update();
			}
		}
	}

private:
	std::vector<Observer*>		m_vObserver;

};

Observer::~Observer() {
	if (NULL != m_pSubject) {
		m_pSubject->UnRegisterObser(this);
	}
}

void Observer::observe(Observable* pObservable) {
	if (NULL == pObservable) {
		return;
	}
	pObservable->registerObser(this);
	m_pSubject = pObservable;
}

class Foo
	: public Observer
{
public:
	Foo() {};
	virtual ~Foo() {}

	virtual void		update() {
		 printf("Foo::update() %p\n", this);
	}
};

int main()
{
	Foo* pFoo = new Foo();
	Observable subject;
	pFoo->observe(&subject);
	subject.notifyObservers();
	delete pFoo;
	subject.notifyObservers();

}
