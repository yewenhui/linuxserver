#include "../Exception.h"
#include <stdio.h>

class Bar
{
public:
	void test() {
		throw Exception("Bar");
	}
};

void foo() {
	Bar b;
	b.test();
}

int main() 
{
	try {
		foo();
	} catch (const Exception& excep) {
		printf("reason: %s\n", excep.what());
		printf("stack trace: %s\n", excep.StackTrace());
	}

	return 0;
}
