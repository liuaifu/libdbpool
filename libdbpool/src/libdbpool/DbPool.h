#pragma once

#include "libdbpool/DbConnection.h"
#include <deque>
#include <mutex>
#include <atomic>
#include <memory>
#include <thread>
#include <chrono>
#include <algorithm>

using namespace std::chrono_literals;

typedef std::shared_ptr<CDbConnection> DbConnectionPtr;

class CDbPool
{
public:
	CDbPool(std::string host, std::string user, std::string passwd, std::string database, int max_connection = 10, unsigned int port = 3306, std::string charset_name = "utf8")
		:m_host(host), m_user(user), m_passwd(passwd), m_database(database), m_port(port), m_charset_name(charset_name)
	{
		static bool initialized = false;
		if (!initialized)
		{
			mysql_library_init(0, NULL, NULL);
			initialized = true;
		}

		m_maxConnection = max_connection;
		m_fnLock = [this]() { m_mutex.lock(); };
		m_fnUnLock = [this]() { m_mutex.unlock(); };
		m_usingCount = 0;
	}

	~CDbPool() {}
	
	/**
	* 设置自己的锁替换默认锁
	* 一般用不上
	*/
	void SetLocker(std::function<void()> fnLock, std::function<void()> fnUnLock)
	{
		m_fnLock = fnLock;
		m_fnUnLock = fnUnLock;
	}

	/**
	* 获取正在使用的连接数
	*/
	size_t GetUsingCount()
	{
		return (size_t)m_usingCount;
	}

	/**
	* 获取空闲的连接数
	*/
	size_t GetIdleCount()
	{
		return m_idleConnections.size();
	}

	/**
	* 获取一个空闲的连接，如果正被使用的连接达到最大值会阻塞直到有空闲的连接释放
	*/
	DbConnectionPtr GetDbConnection()
	{
		m_fnLock();

		//已有空闲连接
		while (!m_idleConnections.empty())
		{
			auto ptr = m_idleConnections.front();
			m_idleConnections.pop_front();
			if (ptr->Ping()) {
				m_usingCount++;
				m_fnUnLock();
				return ptr;
			}
		}

		//没有空闲连接，并且没有达到最大连接数，创建新连接
		size_t idle_size = m_idleConnections.size();
		m_fnUnLock();
		if (idle_size + m_usingCount < m_maxConnection)
		{
			auto ptr = DbConnectionPtr(new CDbConnection(m_host, m_user, m_passwd, m_database, m_port, m_charset_name));
			if (!ptr->Connect()) {
				//无法连接服务器
				ptr.reset();
			}
			else
				m_usingCount++;

			return ptr;
		}

		DbConnectionPtr ptr;
		auto start = std::chrono::high_resolution_clock::now();
		while (true)
		{
			std::this_thread::sleep_for(16ms);
			m_fnLock();
			if (!m_idleConnections.empty())
			{
				ptr = m_idleConnections.front();
				m_idleConnections.pop_front();
				if (ptr->Ping()) {
					m_fnUnLock();
					m_usingCount++;
					break;
				}
				ptr.reset();
			}
			m_fnUnLock();
			auto end = std::chrono::high_resolution_clock::now();
    		std::chrono::duration<double, std::milli> elapsed = end - start;
			if(elapsed.count() >= 5000)
				break;
		}

		return ptr;
	}

	/**
	* 释放一个连接
	*/
	void FreeDbConnection(DbConnectionPtr ptr)
	{
		if(!ptr)return;

		m_fnLock();
		m_usingCount--;
		m_idleConnections.push_back(ptr);
		m_fnUnLock();
	}

	/**
	* 执行非查询，成功返回影响的行数，失败返回-1
	*/
	int ExecuteNonQuery(const char* sql)
	{
		auto cn = GetDbConnection();
		if (!cn)
			return -1;

		bool ret = cn->Query(sql);
		if (!ret)
		{
			FreeDbConnection(cn);
			return -1;
		}

		my_ulonglong affected_rows = cn->GetAffectedRows();
		FreeDbConnection(cn);

		return (int)affected_rows;
	}

private:
	std::function<void()> m_fnLock;
	std::function<void()> m_fnUnLock;

	// 空闲连接
	std::deque<DbConnectionPtr> m_idleConnections;
	std::atomic_int m_usingCount;

	// 资源锁
	std::mutex m_mutex;

	// 最大连接数
	size_t m_maxConnection;

	//数据库信息
	std::string m_host;
	std::string m_user;
	std::string m_passwd;
	unsigned int m_port;
	std::string m_database;
	std::string m_charset_name;
};
