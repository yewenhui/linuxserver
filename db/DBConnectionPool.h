#ifndef _DB_CONNECTION_POOL_H_
#define _DB_CONNECTION_POOL_H_

#include "DBConnection.h"

#include "../thread/ThreadPool.h"

#include <boost/noncopyable.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

#include <deque>


class DBConnectionPool
	: private boost::noncopyable
{
	// typedef boost::function<void()> SqlQueryTask;
	typedef boost::shared_ptr<DBConnection> DBConnectionPtr;

public:
	DBConnectionPool();
	~DBConnectionPool();

	bool				Init(const DBConnectParam& rConnParam, int nPoolSize);

	void				Stop();

	void				ExecuteSql(const char* pStrSql);

private:
	void				runInThread();
	std::string			task();

private:
	DBConnectParam		m_ConnParam;
	ThreadPool			m_ThreadPool;
	std::deque<std::string> m_SqlQueue;
	std::deque<DBConnectionPtr> m_vDBConn;
	bool				m_bRunning;
	MutexLock			m_ConnMutex;
	MutexLock			m_SqlMutex;

};

#endif // _DB_CONNECTION_POOL_H_
