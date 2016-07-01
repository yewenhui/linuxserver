#include "../DBConnectionPool.h"

#include "../../datetime/TimeStamp.h"

#include <stdio.h>

DBConnectionPool pool;

void InsertBenchTest() {
	TimeStamp start(TimeStamp::Now());
	for (int n = 0; n < 10000; ++ n) {
		char strSql[128];
		sprintf(strSql, "CALL SP_INSERT_PLAYERTABLE(\'%s\', %d);", "nihao", n);
		// sprintf(strSql, "INSERT INTO playertable(name, age) VALUES(\'%s\', %u);", "nihao", n);
		pool.ExecuteSql(strSql);
	}

	pool.Stop();
	TimeStamp end(TimeStamp::Now());
	printf("InsertBenchTest: Insert 10000 record cost times -> %f\n", TimeDifference(end, start));
}

int main() {
	DBConnectParam connectParam("127.0.0.1", 3306, "yewenhui", "ye86412596", "test");
	pool.Init(connectParam, 4);

	InsertBenchTest();
	return 0;
}