#ifndef _DB_RECORD_H_
#define _DB_RECORD_H_

#include <mysql/mysql.h>
#include <boost/shared_ptr.hpp>

#include <map>
#include <string>

class DBRecordSet;

class DBRecord
{
	typedef std::map<std::string, unsigned int> FieldsNameIndexMap;

public:
	DBRecord(DBRecordSet* pRecordSet, MYSQL_RES* pRes, MYSQL_ROW row);
	~DBRecord();

	const char*			GetFieldValueByIdx(unsigned int unIndex);
	const char*			GetFiledValueByName(const char* pFiledName);

	int					GetFieldLengthByIdx(unsigned int unIndex);
	int					GetFiledLengthByName(const char* pFiledName);

private:
	void				fillFieldsNameIndxMap();

private:
	DBRecordSet*		m_pRecordSet;
	MYSQL_RES*			m_pRes;
	MYSQL_ROW			m_Row;
	unsigned long*		m_pFieldsLength;

	FieldsNameIndexMap	m_mpNameIdx;

};

typedef boost::shared_ptr<DBRecord> DBRecordPtr;

#endif // _DB_RECORD_H_
