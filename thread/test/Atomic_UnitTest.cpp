#include "../Atomic.h"
#include <assert.h>

int main()
{
	AtomicInt32 a0;
	assert(a0.Get() == 0);
	assert(a0.GetAndAdd(1) == 0);
	assert(a0.Get() == 1);
	assert(a0.AddAndGet(2) == 3);
	assert(a0.Get() == 3);
	assert(a0.IncrementAndGet() == 4);
	assert(a0.Get() == 4);
	assert(a0.GetAndSet(100) == 4);
	assert(a0.Get() == 100);

	AtomicInt64 a1;
	assert(a1.Get() == 0);
	assert(a1.GetAndAdd(1) == 0);
	assert(a1.Get() == 1);
	assert(a1.AddAndGet(2) == 3);
	assert(a1.Get() == 3);
	assert(a1.IncrementAndGet() == 4);
	assert(a1.Get() == 4);
	assert(a1.GetAndSet(100) == 4);
	assert(a1.Get() == 100);
}
