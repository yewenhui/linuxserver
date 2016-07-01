#include "DBConnection.h"

#include <assert.h>

#include <stdio.h>

DBConnection::DBConnection()
	: m_pMysqlConn(0), m_bIsUsed(false)
{

}

DBConnection::~DBConnection() {
	Release();
// 	if (NULL != m_pMysqlConn) {
// 		assert(false);
// 		mysql_close(m_pMysqlConn);
// 	}
}

bool DBConnection::Connect(const DBConnectParam& rConnParam) {
	if(NULL != m_pMysqlConn) {
		assert(false);
		return false;
	}
	// 参数NULL分配内存，初始化，并且需要mysql_close关闭连接
	m_pMysqlConn = mysql_init(NULL);
	if (NULL == m_pMysqlConn) {
		assert(false);
		return false;
	}

	m_ConnParam = rConnParam;
	bool bConnected = NULL != mysql_real_connect(m_pMysqlConn
								, rConnParam.strIp.c_str()
								, rConnParam.strUserName.c_str()
								, rConnParam.strPassWord.c_str()
								, rConnParam.strDBName.c_str()
								, rConnParam.usPort
								, NULL
								, 0);
	if (!bConnected) {
		// unsigned int unErrno = mysql_errno(m_pMysqlConn);
		// const char* pStrErrMsg = mysql_error(m_pMysqlConn);
		assert(false);
		return false;
	}

	return true;
}

bool DBConnection::ReConnect() {
	Release();
	return Connect(m_ConnParam);
}

void DBConnection::Release() {
	if (NULL != m_pMysqlConn) {
		mysql_close(m_pMysqlConn);
		m_pMysqlConn = NULL;
	}
}

bool DBConnection::IsConnected() {
	return NULL != m_pMysqlConn;
}

DBRecordSetPtr DBConnection::ExecuteSql(const char* pStrSql) {
	if (NULL == pStrSql) {
		assert(false);
		return DBRecordSetPtr();
	}

	if (!IsConnected()) {
		assert(false);
		return DBRecordSetPtr();
	}

	if (0 != mysql_query(m_pMysqlConn, pStrSql)) {
		const char* pStrErrMsg = mysql_error(m_pMysqlConn);
		printf("mysql err: %s\n", pStrErrMsg);
		assert(false);
		return DBRecordSetPtr();
	}

	MYSQL_RES* pRes = mysql_store_result(m_pMysqlConn);
	if (NULL == pRes) {
		return DBRecordSetPtr();
	}

	return DBRecordSetPtr(new DBRecordSet(pRes));
}

void DBConnection::BeginTransaction() {
	if (!IsConnected()) {
		return;
	}
	mysql_autocommit(m_pMysqlConn, 0);
}

void DBConnection::CommitTransaction() {
	if (!IsConnected()) {
		return;
	}
	mysql_commit(m_pMysqlConn);
	mysql_autocommit(m_pMysqlConn, 1);
}

void DBConnection::RollBackTransaction() {
	if (!IsConnected()) {
		return;
	}
	mysql_rollback(m_pMysqlConn);
	mysql_autocommit(m_pMysqlConn, 1);
}
