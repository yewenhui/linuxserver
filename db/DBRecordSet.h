#ifndef _DB_RECONRD_SET_H_
#define _DB_RECONRD_SET_H_

#include "DBRecord.h"

#include <mysql/mysql.h>

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

// struct DBRecordPtr;

class DBRecordSet
	: private boost::noncopyable
{
public:
	DBRecordSet(MYSQL_RES* pRes);
	~DBRecordSet();

	// 记录个数
	unsigned int		GetRecordNum();
	// 字段个数
	unsigned int		GetFieldCount();

	DBRecordPtr			GetNextRecord();

private:
	MYSQL_RES*			m_pRes;
	MYSQL_ROW			m_Row;

};

typedef boost::shared_ptr<DBRecordSet> DBRecordSetPtr;

#endif // _DB_RECONRD_SET_H_
