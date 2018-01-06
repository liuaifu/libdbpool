#pragma once

#include "DbConnection.h"
#include <vector>
#include <boost/thread/mutex.hpp>

class CDbPool
{
public:
	CDbPool(std::string host, std::string user, std::string passwd, std::string database, unsigned int port = 3306);
	~CDbPool();

private:
	// 空闲连接
	std::vector<CDbConnection*> m_vtIdleConnections;

	// 正在被使用的连接
	std::vector<CDbConnection*> m_vtBusyConnections;

	// 资源锁
	boost::mutex m_mutex;

	// 最大连接数
	size_t m_maxConnection;

	//数据库信息
	std::string m_host;
	std::string m_user;
	std::string m_passwd;
	unsigned int m_port;
	std::string m_database;

public:
	// 获取一个空闲的连接，如果正被使用的连接达到最大值会阻塞直到有空闲的连接释放
	CDbConnection* GetDbConnection();

	// 获取一个空闲的连接，如果正被使用的连接达到最大值会阻塞直到有空闲的连接释放
	void FreeDbConnection(CDbConnection* pConnection);

	/**
	* 执行非查询，成功返回影响的行数，失败返回-1
	*/
	int ExecuteNonQuery(const char* sql);
};