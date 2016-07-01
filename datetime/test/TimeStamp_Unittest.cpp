#include "../TimeStamp.h"

#include <vector>
#include <stdio.h>

// void PassByConstReference(const TimeStamp& rStamp) {
// 	printf("%s\n", rStamp.ToString().c_str());
// }
// 
// void PassByValue(TimeStamp stamp) {
// 	printf("%s\n", stamp.ToString().c_str());
// }

void BenchMark() {
	const unsigned int NUMBER = 1000 * 1000;

	std::vector<TimeStamp> vStamp;
	for (size_t n = 0; n < NUMBER; ++n) {
		vStamp.push_back(TimeStamp::Now());
	}

	printf("%s\n", vStamp.front().ToString().c_str());
	printf("%s\n", vStamp.back().ToString().c_str());
	printf("%f\n", TimeDifference(vStamp.back(), vStamp.front()));

	int increments[100] = { 0 };
	int64_t start = vStamp.front().GetMicroSecondsSinceEpoch();
	for (size_t n = 1; n < NUMBER; ++n) {
		int64_t next = vStamp[n].GetMicroSecondsSinceEpoch();
		int inc = next - start;
		start = next;
		if (inc < 0) {
			printf("reverse!\n");
		} else if (inc < 100) {
			++increments[inc];
		} else {
			printf("big gap %d\n", inc);
		}
	}

	for (unsigned int n = 0; n < 100; ++n) {
		printf("%2d: %d\n", n, increments[n]);
	}

}

int main(int argc, char* argv[]) {
	TimeStamp timeStamp(TimeStamp::Now());
	printf("%s\n", timeStamp.ToString().c_str());
	BenchMark();

	// return 0;
}