#include "../Mutex.h"

#include <assert.h>
#include <stdio.h>
#include <map>

#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

using namespace std;

class Stock 
	: private boost::noncopyable
{
public:
	Stock(const string& name)
		: m_StrName(name)
	{
		printf(" Stock[%p] %s\n", this, m_StrName.c_str());
	}

	~Stock() {
		printf("~Stock[%p] %s\n", this, m_StrName.c_str());
	}

	const string&		key() const { return m_StrName; }

private:
	string				m_StrName;

};

namespace version1
{

// 有问题的代码
class StockFactory
	: private boost::noncopyable
{
public:
	boost::shared_ptr<Stock> get(const string& strKey) {
		MutexLockGuard lock(m_Mutex);
		boost::shared_ptr<Stock>& pStock = m_mpStock[strKey];
		if (!pStock) {
			pStock.reset(new Stock(strKey));
		}
		return pStock;
	}

private:
	mutable MutexLock		m_Mutex;
	map<string, boost::shared_ptr<Stock> > m_mpStock;

};

}

namespace version2
{

class StockFactory
	: private boost::noncopyable
{
public:
	boost::shared_ptr<Stock> get(const string& strKey) {
		boost::shared_ptr<Stock> pStock;
		MutexLockGuard lock(m_Mutex);
		boost::weak_ptr<Stock>& wkStock = m_mpStock[strKey];
		pStock = wkStock.lock();
		if (!pStock) {
			pStock.reset(new Stock(strKey));
			wkStock = pStock;
		}
		return pStock;
	}

private:
	mutable MutexLock		m_Mutex;
	map<string, boost::weak_ptr<Stock> > m_mpStock;

};

}

namespace version3
{

class StockFactory
	: private boost::noncopyable
{
public:
	boost::shared_ptr<Stock> get(const string& strKey) {
		boost::shared_ptr<Stock> pStock;
		MutexLockGuard lock(m_Mutex);
		boost::weak_ptr<Stock>& wkStock = m_mpStock[strKey];
		pStock = wkStock.lock();
		if (!pStock) {
			pStock.reset(new Stock(strKey)
				, boost::bind(&StockFactory::deleteStock, this, _1));
			wkStock = pStock;
		}
		return pStock;
	}

private:
	void deleteStock(Stock* pStock) {
		printf("deleteStock [%p] \n", pStock);
		if (pStock) {
			MutexLockGuard lock(m_Mutex);
			m_mpStock.erase(pStock->key());
		}
		delete pStock; // 
	}

private:
	mutable MutexLock		m_Mutex;
	map<string, boost::weak_ptr<Stock> > m_mpStock;

};

}

namespace version4
{

class StockFactory
	: public boost::enable_shared_from_this<StockFactory>
	, private boost::noncopyable
{
public:
	boost::shared_ptr<Stock> get(const string& strName) {
		boost::shared_ptr<Stock> pStock;
		MutexLockGuard lock(m_Mutex);
		boost::weak_ptr<Stock>& wkStock = m_mpStock[strName];
		pStock = wkStock.lock();
		if (NULL != pStock) {
			pStock.reset(new Stock(strName)
				, boost::bind(&StockFactory::deleteStock, shared_from_this(), _1));
			wkStock = pStock;
		}

		return pStock;
	}

private:
	void deleteStock(Stock* pStock) {
		printf("deleteStock [%p]\n", pStock);
		if (pStock) {
			MutexLockGuard lock(m_Mutex);
			m_mpStock.erase(pStock->key());
		}
		delete pStock; // 
	}

private:
	mutable MutexLock		m_Mutex;
	map<string, boost::weak_ptr<Stock> > m_mpStock;

};

}

class StockFactory
	: public boost::enable_shared_from_this<StockFactory>
	, private boost::noncopyable
{
public:
	boost::shared_ptr<Stock> get(const string& rStrName) {
 		boost::shared_ptr<Stock> pStock;
		MutexLockGuard lock(m_Mutex);
 		boost::weak_ptr<Stock>& wkStock = m_mpStock[rStrName];
		pStock = wkStock.lock();
		if (NULL != pStock) {
			pStock.reset(new Stock(rStrName)
				, boost::bind(&StockFactory::weakDeleteCallBack
								, boost::weak_ptr<StockFactory>(shared_from_this())
								, _1));
			wkStock = pStock;
		}
 		return pStock;
	}

private:
	static void			weakDeleteCallBack(const boost::weak_ptr<StockFactory>& wkFactory
		, Stock* pStock) 
	{
		printf("weakDeleteStock[%p]\n", pStock);
		boost::shared_ptr<StockFactory> pFactory(wkFactory.lock());
		if (NULL != pFactory) {
			 pFactory->removeStock(pStock);
		} else {
			printf("factory died.\n");
		}
		delete pStock;
	}

	void removeStock(Stock* pStock) {
		if (NULL != pStock) {
			MutexLockGuard lock(m_Mutex);
			m_mpStock.erase(pStock->key());
		}
	}

private:
	mutable MutexLock		m_Mutex;
	map<string, boost::weak_ptr<Stock> > m_mpStock;

};

void testLongLifeFactory() {
	boost::shared_ptr<StockFactory> factory(new StockFactory);
	{
		boost::shared_ptr<Stock> stock = factory->get("testLongLifeFactory");
		boost::shared_ptr<Stock> stock2 = factory->get("testLongLifeFactory");
		assert(stock == stock2);
	}
}

void testShortLifeFactory() {
	boost::shared_ptr<Stock> pStock;
	{
		boost::shared_ptr<StockFactory> factory(new StockFactory);
		pStock = factory->get("testShortLifeFactory");
		boost::shared_ptr<Stock> stock2 = factory->get("testShortLifeFactory");
		assert(pStock == stock2);
	}
}

int main()
{
	version1::StockFactory sf1;
	version2::StockFactory sf2;
	version3::StockFactory sf3;
 	boost::shared_ptr<version3::StockFactory> sf4(new version3::StockFactory);
 	boost::shared_ptr<StockFactory> sf5(new StockFactory);

	{
		boost::shared_ptr<Stock> s1 = sf1.get("stock1");
	}

	{
		boost::shared_ptr<Stock> s2 = sf2.get("stock2");
	}

	{
		boost::shared_ptr<Stock> s3 = sf3.get("stock3");
	}

	{
		boost::shared_ptr<Stock> s4 = sf4->get("stock4");
	}

	{
		boost::shared_ptr<Stock> s5 = sf5->get("stock5");
	}

	testLongLifeFactory();
	testShortLifeFactory();
}
