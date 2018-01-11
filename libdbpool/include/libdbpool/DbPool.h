#pragma once

#include "libdbpool/DbConnection.h"
#include <vector>
#include <boost/thread/mutex.hpp>

class CDbPool
{
public:
	CDbPool(std::string host, std::string user, std::string passwd, std::string database, unsigned int port = 3306, std::string charset_name = "utf8");
	~CDbPool();
	
	/**
	* 设置自己的锁替换默认锁
	* 一般用不上
	*/
	void SetLocker(std::function<void()> fnLock, std::function<void()> fnUnLock);

private:
	std::function<void()> m_fnLock;
	std::function<void()> m_fnUnLock;

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
	std::string m_charset_name;

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
