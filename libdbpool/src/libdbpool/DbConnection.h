#pragma once

#include <string>
#include <vector>
#ifdef WIN32
#include <mysql.h>
#else
#include <mysql/mysql.h>
#endif
#include <assert.h>

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
	CDbConnection(std::string host, std::string user, std::string passwd, std::string database, unsigned int port, std::string charset_name)
		:m_host(host), m_user(user), m_passwd(passwd), m_database(database), m_port(port), m_affected_rows(0)
	{
		MYSQL* mysql = mysql_init(&m_mysql);
		assert(mysql != NULL);

		mysql_options(mysql, MYSQL_SET_CHARSET_NAME, charset_name.c_str());
		my_bool value = 1;
		mysql_options(&m_mysql, MYSQL_OPT_RECONNECT, &value);
	}

	~CDbConnection()
	{
		mysql_close(&m_mysql);
	}

	inline bool Ping() { return mysql_ping(&m_mysql) == 0; }

	bool Query(const char* sql)
	{
		m_affected_rows = 0;
		m_result.clear();

		if (mysql_query(&m_mysql, sql))
			return false;

		m_affected_rows = mysql_affected_rows(&m_mysql);

		MYSQL_RES *pRes = mysql_store_result(&m_mysql);
		if (pRes == NULL)
			return true;	//非查询

		while (MYSQL_ROW row = mysql_fetch_row(pRes))
		{
			CDbRow db_row;
			for (size_t i = 0; i < mysql_num_fields(pRes); i++)
			{
				db_row.AddColumn(row[i] ? row[i] : "null");
			}
			m_result.AddRow(db_row);
		}
		mysql_free_result(pRes);

		//清除其余结果集
		while (!mysql_next_result(&m_mysql)) {
			pRes = mysql_store_result(&m_mysql);
			if (pRes != NULL)
				mysql_free_result(pRes);
		}

		return true;
	}

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

	inline bool Connect()
	{
		return NULL != mysql_real_connect(&m_mysql, m_host.c_str(), m_user.c_str(), m_passwd.c_str(), m_database.c_str(), m_port, NULL, 0/*CLIENT_MULTI_RESULTS*/);
	}

private:
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
