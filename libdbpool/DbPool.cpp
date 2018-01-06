#include "DbPool.h"
#include <algorithm>
#include <boost/thread/thread_time.hpp>
#ifdef WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

CDbPool::CDbPool(std::string host, std::string user, std::string passwd, std::string database, unsigned int port/* = 3306*/)
	:m_maxConnection(10), m_host(host), m_user(user), m_passwd(passwd), m_database(database), m_port(port)
{
	static bool initialized = false;
	if (!initialized)
	{
		mysql_library_init(0, NULL, NULL);
		initialized = true;
	}
}


CDbPool::~CDbPool()
{
}


// 获取一个空闲的连接，会阻塞直到有空闲的连接
CDbConnection* CDbPool::GetDbConnection()
{
	m_mutex.lock();

	//已有空闲连接
	if (m_vtIdleConnections.size() > 0)
	{
		CDbConnection* pConnection = m_vtIdleConnections.back();
		m_vtIdleConnections.pop_back();
		m_vtBusyConnections.push_back(pConnection);
		m_mutex.unlock();
		return pConnection;
	}

	//没有空闲连接，并且没有达到最大连接数，创建新连接
	if (m_vtIdleConnections.size() + m_vtBusyConnections.size() < m_maxConnection)
	{
		CDbConnection* pConnection = new CDbConnection("localhost", "root", "123", "test", 3306);
		m_vtBusyConnections.push_back(pConnection);
		m_mutex.unlock();
		return pConnection;
	}

	CDbConnection* pConnection = NULL;
	while (true)
	{
		m_mutex.unlock();
#ifdef WIN32
		Sleep(16);
#else
		usleep(16000);
#endif
		m_mutex.lock();
		if (m_vtIdleConnections.size() > 0)
		{
			pConnection = m_vtIdleConnections.back();
			m_vtIdleConnections.pop_back();
			m_vtBusyConnections.push_back(pConnection);
			break;
		}
	}

	m_mutex.unlock();

	return pConnection;
}

void CDbPool::FreeDbConnection(CDbConnection* pConnection)
{
	m_mutex.lock();
	std::vector<CDbConnection*>::iterator it = std::find(m_vtBusyConnections.begin(), m_vtBusyConnections.end(), pConnection);
	if(it != m_vtBusyConnections.end())
		m_vtBusyConnections.erase(it);
	m_vtIdleConnections.push_back(pConnection);
	m_mutex.unlock();
}

int CDbPool::ExecuteNonQuery(const char* sql)
{
	CDbConnection *pConnection = GetDbConnection();
	if (pConnection == NULL)
		return -1;

	bool ret = pConnection->Query(sql);
	if (!ret)
	{
		FreeDbConnection(pConnection);
		return -1;
	}
	
	my_ulonglong affected_rows = pConnection->GetAffectedRows();
	FreeDbConnection(pConnection);

	return (int)affected_rows;
}