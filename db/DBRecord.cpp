#include "DBRecord.h"
#include "DBRecordSet.h"
#include <assert.h>

DBRecord::DBRecord(DBRecordSet* pRecordSet, MYSQL_RES* pRes, MYSQL_ROW row)
	: m_pRecordSet(pRecordSet), m_pRes(pRes), m_Row(row)
{
	assert(NULL != m_pRes);
	assert(NULL != m_Row);

	m_pFieldsLength = mysql_fetch_lengths(m_pRes);
	assert(NULL != m_pFieldsLength);

	fillFieldsNameIndxMap();
}

DBRecord::~DBRecord() {

}

const char* DBRecord::GetFieldValueByIdx(unsigned int unIndex) {
	if (unIndex >= m_pRecordSet->GetFieldCount()) {
		return NULL;
	}

	return m_Row[unIndex];
}

const char* DBRecord::GetFiledValueByName(const char* pFiledName) {
	if (NULL == pFiledName) {
		assert(false);
		return NULL;
	}

	FieldsNameIndexMap::const_iterator itor = m_mpNameIdx.find(pFiledName);
	if (m_mpNameIdx.end() != itor) {
		return GetFieldValueByIdx(itor->second);
	}

	return NULL;
}

int DBRecord::GetFieldLengthByIdx(unsigned int unIndex) {
	if (unIndex >= m_pRecordSet->GetFieldCount()) {
		return NULL;
	}

	return m_pFieldsLength[unIndex];
}

int DBRecord::GetFiledLengthByName(const char* pFiledName) {
	if (NULL == pFiledName) {
		assert(false);
		return NULL;
	}

	FieldsNameIndexMap::const_iterator itor = m_mpNameIdx.find(pFiledName);
	if (m_mpNameIdx.end() != itor) {
		return GetFieldLengthByIdx(itor->second);
	}

	return NULL;
}

void DBRecord::fillFieldsNameIndxMap() {
	MYSQL_FIELD* pField = NULL;
	unsigned int unIdx = 0;
	mysql_field_seek(m_pRes, 0);
	pField = mysql_fetch_field(m_pRes);
	while (NULL != pField) {
		m_mpNameIdx[pField->name] = unIdx;
		++ unIdx;
		pField = mysql_fetch_field(m_pRes);
	}
}
