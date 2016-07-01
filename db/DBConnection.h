#ifndef _DB_CONNECTION_H_ 
#define _DB_CONNECTION_H_

#include "DBDef.h"
#include "DBRecordSet.h"

#include <mysql/mysql.h>

#include <boost/noncopyable.hpp>

class DBConnection
	: private boost::noncopyable
{
	enum eDBConnectState {

	};

public:
	DBConnection();
	~DBConnection();

	bool			Connect(const DBConnectParam& rConnParam);
	bool			ReConnect();
	void			Release();

	bool			IsConnected();

	DBRecordSetPtr	ExecuteSql(const char* pStrSql);

	void			BeginTransaction();
	void			CommitTransaction();
	void			RollBackTransaction();

	bool			IsUsed() {
		return m_bIsUsed;
	}
	void			SetUsed(bool bIsUsed) {
		m_bIsUsed = bIsUsed;
	}

private:
	MYSQL*			m_pMysqlConn;
	DBConnectParam	m_ConnParam;

	volatile bool	m_bIsUsed;

};

#endif // _DB_CONNECTION_H_
