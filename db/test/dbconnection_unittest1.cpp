#include "../DBConnection.h"
#include "../DBRecordSet.h"

#include <stdio.h>
#include "../../datetime/TimeStamp.h"

DBConnection dbConnection;

void InsertBenchTest() {
	TimeStamp start(TimeStamp::Now());
	for (size_t n = 0; n < 100000; ++ n) {
		char strSql[128];
		sprintf(strSql, "CALL SP_INSERT_PLAYERTABLE(\'%s\', %u);", "nihao", n);
		// sprintf(strSql, "INSERT INTO playertable(name, age) VALUES(\'%s\', %u);", "nihao", n);
		dbConnection.ExecuteSql(strSql);
	}

	TimeStamp end(TimeStamp::Now());
	printf("InsertBenchTest: Insert 10000 record cost times -> %f\n", TimeDifference(end, start));
}

void SelectBenchTest() {
	TimeStamp start(TimeStamp::Now());

	std::string strSql = "CALL SP_SELECT_PLAYERTABLE();";
	DBRecordSetPtr pRecordSet = dbConnection.ExecuteSql(strSql.c_str());
	if (NULL == pRecordSet) {
		dbConnection.Release();
		return;
	}

	printf("Record Num : %d\n", pRecordSet->GetRecordNum());

	DBRecordPtr pRecord = pRecordSet->GetNextRecord();
	while (NULL != pRecord) {
		printf("%s\t", pRecord->GetFiledValueByName("id"));
		printf("%s\t", pRecord->GetFiledValueByName("name"));
		printf("%s\t", pRecord->GetFiledValueByName("age"));
		printf("\n");

		pRecord = pRecordSet->GetNextRecord();
	}

	TimeStamp end(TimeStamp::Now());
	printf("InsertBenchTest %f\n", TimeDifference(end, start));
}

int main() {
	DBConnectParam connectParam("127.0.0.1", 3306, "yewenhui", "ye86412596", "test");
	dbConnection.Connect(connectParam);
	assert(dbConnection.IsConnected());

	// InsertBenchTest();
	SelectBenchTest();

	dbConnection.Release();
	return 0;
}
