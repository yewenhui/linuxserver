#ifndef _ATOMIC_H_
#define _ATOMIC_H_

#include <stdint.h>
#include <boost/noncopyable.hpp>

template<typename T>
class AtomicIntegerT
	: private boost::noncopyable
{
public:
	AtomicIntegerT()
		: m_value(0)
	{
	}

	AtomicIntegerT(const AtomicIntegerT& that)
		: m_value(that.Get())
	{
	}

	AtomicIntegerT& operator=(const AtomicIntegerT& that) {
		GetAndSet(that.Get());
		return *this;
	}
	
	T			Get() const {
		return __sync_val_compare_and_swap(const_cast<volatile T*>(&m_value), 0, 0);
	}

	T			GetAndAdd(T x) {
		return __sync_fetch_and_add(&m_value, x);
	}

	T			AddAndGet(T x) {
		return GetAndAdd(x) + x;
	}

	T			IncrementAndGet() {
		return AddAndGet(1);
	}

	void		Add(T x) {
		GetAndAdd(x);
	}

	void		Increment() {
		IncrementAndGet();
	}

	void		Decrement() {
		GetAndAdd(-1);
	}

	T			GetAndSet(T newValue) {
		return __sync_lock_test_and_set(&m_value, newValue);
	}

private:
	volatile T	m_value;

};

typedef AtomicIntegerT<int32_t> AtomicInt32;
typedef AtomicIntegerT<int64_t> AtomicInt64;

#endif // _ATOMIC_H_