#include "DBConnectionPool.h"

#include <boost/bind.hpp>
#include "../thread/CountDownLatch.h"

DBConnectionPool::DBConnectionPool() 
	: m_ThreadPool("DB Connection Thread Pool")
	, m_bRunning(false)
{

}

DBConnectionPool::~DBConnectionPool() {
	assert(!m_bRunning);
}

bool DBConnectionPool::Init(const DBConnectParam& rConnParam, int nPoolSize) {
	if (nPoolSize < 0) {
		assert(false);
		return false;
	}

	if (m_bRunning) {
		return false;
	}

	m_ConnParam = rConnParam;	
	{
		MutexLockGuard lock(m_ConnMutex);
		for (int n = 0; n < nPoolSize; ++ n) {
			DBConnectionPtr pConn(new DBConnection);
			pConn->Connect(m_ConnParam);
			m_vDBConn.push_back(pConn);
		}
	}

	m_bRunning = true;
	m_ThreadPool.Start(nPoolSize);
	return true;
}

void DBConnectionPool::Stop() {
	if (m_bRunning) {
		m_bRunning = false;

		CountDownLatch latch(1);
		m_ThreadPool.Run(boost::bind(&CountDownLatch::CountDown, &latch));
		latch.Wait();

		m_ThreadPool.Stop();
	}
}

void DBConnectionPool::ExecuteSql(const char* pStrSql) {
	if (NULL == pStrSql) {
		return;
	}

	{
		MutexLockGuard lock(m_SqlMutex);
		m_SqlQueue.push_back(pStrSql);
	}
	
	m_ThreadPool.Run(boost::bind(&DBConnectionPool::runInThread, this));
}

void DBConnectionPool::runInThread() {
	std::string strSql(task());
	assert(!strSql.empty());

	DBConnectionPtr pDBConn;
	{
		MutexLockGuard lock(m_ConnMutex);
		assert(!m_vDBConn.empty());
		pDBConn = m_vDBConn.front();
		m_vDBConn.pop_front();
	}

	assert(NULL != pDBConn);
	pDBConn->ExecuteSql(strSql.c_str());

	{
		MutexLockGuard lock(m_ConnMutex);
		m_vDBConn.push_back(pDBConn);
	}
}

std::string DBConnectionPool::task() {
	MutexLockGuard lock(m_SqlMutex);
	std::string strRet;
	if (!m_SqlQueue.empty()) {
		strRet = m_SqlQueue.front();
		m_SqlQueue.pop_front();
	}

	return strRet;
}
