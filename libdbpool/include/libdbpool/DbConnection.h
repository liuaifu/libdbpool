#pragma once

#ifdef WIN32
#include <WinSock2.h>
#endif
#include <string>
#include <vector>
#ifdef WIN32
#include <mysql.h>
#else
#include <mysql/mysql.h>
#endif

class CDbRow
{
public:
	CDbRow() {}
	~CDbRow() {}

	const char* operator[](size_t i)
	{
		if (i >= m_data.size())
			return NULL;

		return m_data[i].c_str();
	}

	size_t size()
	{
		return m_data.size();
	}

	void AddColumn(std::string column)
	{
		m_data.push_back(column);
	}

private:
	std::vector<std::string> m_data;
};

class CDbResult
{
public:
	CDbResult() {}
	~CDbResult() {}

	void AddRow(CDbRow& row)
	{
		m_data.push_back(row);
	}

	void clear()
	{
		m_data.clear();
	}

	CDbRow operator[](size_t i)
	{
		if (i >= m_data.size())
			return CDbRow();

		return m_data[i];
	}

	size_t size()
	{
		return m_data.size();
	}

private:
	std::vector<CDbRow> m_data;
};

class CDbConnection
{
public:
	CDbConnection(std::string host, std::string user, std::string passwd, std::string database, unsigned int port, std::string charset_name);
	~CDbConnection();

	bool Query(const char* sql);
	inline const CDbResult& GetResult()
	{
		return m_result;
	}

	inline my_ulonglong GetAffectedRows()
	{
		return m_affected_rows;
	}

	inline std::string GetLastError()
	{
		return mysql_error(&m_mysql);
	}

private:
	inline bool Connect();

	MYSQL m_mysql;
	std::string m_host;
	std::string m_user;
	std::string m_passwd;
	unsigned int m_port;
	std::string m_database;
	std::string m_charset_name;
	CDbResult m_result;
	my_ulonglong m_affected_rows;
};

