#include "DBRecordSet.h"
#include "DBRecord.h"

DBRecordSet::DBRecordSet(MYSQL_RES* pRes) 
	: m_pRes(pRes)
{
	assert(NULL != m_pRes);
}

DBRecordSet::~DBRecordSet() {
	if (NULL != m_pRes) {
		mysql_free_result(m_pRes);
	}
}

unsigned int DBRecordSet::GetRecordNum() {
	return NULL != m_pRes ? mysql_num_rows(m_pRes) : 0;
}

unsigned int DBRecordSet::GetFieldCount() {
	return NULL != m_pRes ? mysql_num_fields(m_pRes) : 0;
}

DBRecordPtr DBRecordSet::GetNextRecord() {
	if (NULL == m_pRes) {
		return DBRecordPtr();
	}

	m_Row = mysql_fetch_row(m_pRes);
	if (NULL == m_Row) {
		return DBRecordPtr();
	}

	return DBRecordPtr(new DBRecord(this, m_pRes, m_Row));
}
