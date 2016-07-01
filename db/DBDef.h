#ifndef _DB_DEF_H_ 
#define _DB_DEF_H_

#include <string>

struct DBConnectParam {
	std::string		strIp;
	unsigned short	usPort;
	std::string		strUserName;
	std::string		strPassWord;
	std::string		strDBName;

	DBConnectParam(std::string ip = ""
				, unsigned short port = 0
				, std::string userName = ""
				, std::string passWord = ""
				, std::string dbName = "")
		: strIp(ip)
		, usPort(port)
		, strUserName(userName)
		, strPassWord(passWord)
		, strDBName(dbName)
	{

	}

};

#endif //_DB_DEF_H_
