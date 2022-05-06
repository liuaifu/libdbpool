#pragma once

#include <string>
#include <vector>
#include <string.h>
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
		int value = 1;
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

	template<typename ...Args>
	inline bool Query(const char* sql, const Args& ...args) {
		std::vector<MYSQL_BIND> binds;
		std::vector<unsigned long> lengths;
		return Query(sql, binds, lengths, args...);
	}

	template<typename ...Args>
	inline bool Query(const char* sql, std::vector<MYSQL_BIND>& binds, std::vector<unsigned long>& lengths, short value, const Args& ...args) {
		lengths.push_back(0);

		MYSQL_BIND bind;
		memset(&bind, 0, sizeof(MYSQL_BIND));
		bind.buffer_type = MYSQL_TYPE_SHORT;
		bind.buffer = (char*)&value;
		bind.is_null = 0;
		bind.length = 0;
		binds.push_back(bind);
		return Query(sql, binds, lengths, args...);
	}

	template<typename ...Args>
	inline bool Query(const char* sql, std::vector<MYSQL_BIND>& binds, std::vector<unsigned long>& lengths, unsigned short value, const Args& ...args) {
		lengths.push_back(0);

		MYSQL_BIND bind;
		memset(&bind, 0, sizeof(MYSQL_BIND));
		bind.buffer_type = MYSQL_TYPE_SHORT;
		bind.buffer = (char*)&value;
		bind.is_null = 0;
		bind.is_unsigned = true;
		bind.length = 0;
		binds.push_back(bind);
		return Query(sql, binds, lengths, args...);
	}

	template<typename ...Args>
	inline bool Query(const char* sql, std::vector<MYSQL_BIND>& binds, std::vector<unsigned long>& lengths, int value, const Args& ...args) {
		lengths.push_back(0);

		long tmp_value = value;

		MYSQL_BIND bind;
		memset(&bind, 0, sizeof(MYSQL_BIND));
		bind.buffer_type = MYSQL_TYPE_LONG;
		bind.buffer = (char*)&tmp_value;
		bind.is_null = 0;
		bind.length = 0;
		binds.push_back(bind);
		return Query(sql, binds, lengths, args...);
	}

	template<typename ...Args>
	inline bool Query(const char* sql, std::vector<MYSQL_BIND>& binds, std::vector<unsigned long>& lengths, unsigned int value, const Args& ...args) {
		lengths.push_back(0);

		unsigned long tmp_value = value;

		MYSQL_BIND bind;
		memset(&bind, 0, sizeof(MYSQL_BIND));
		bind.buffer_type = MYSQL_TYPE_LONG;
		bind.buffer = (char*)&tmp_value;
		bind.is_null = 0;
		bind.is_unsigned = true;
		bind.length = 0;
		binds.push_back(bind);
		return Query(sql, binds, lengths, args...);
	}

	// template<typename ...Args>
	// inline bool Query(const char* sql, std::vector<MYSQL_BIND>& binds, std::vector<unsigned long>& lengths, long value, const Args& ...args) {
	// 	lengths.push_back(0);

	// 	MYSQL_BIND bind;
	// 	memset(&bind, 0, sizeof(MYSQL_BIND));
	// 	bind.buffer_type = MYSQL_TYPE_LONG;
	// 	bind.buffer = (char*)&value;
	// 	bind.is_null = 0;
	// 	bind.length = 0;
	// 	binds.push_back(bind);
	// 	return Query(sql, binds, lengths, args...);
	// }

	// template<typename ...Args>
	// inline bool Query(const char* sql, std::vector<MYSQL_BIND>& binds, std::vector<unsigned long>& lengths, unsigned long value, const Args& ...args) {
	// 	lengths.push_back(0);

	// 	MYSQL_BIND bind;
	// 	memset(&bind, 0, sizeof(MYSQL_BIND));
	// 	bind.buffer_type = MYSQL_TYPE_LONG;
	// 	bind.buffer = (char*)&value;
	// 	bind.is_null = 0;
	// 	bind.is_unsigned = true;
	// 	bind.length = 0;
	// 	binds.push_back(bind);
	// 	return Query(sql, binds, lengths, args...);
	// }

	template<typename ...Args>
	inline bool Query(const char* sql, std::vector<MYSQL_BIND>& binds, std::vector<unsigned long>& lengths, int64_t value, const Args& ...args) {
		lengths.push_back(0);

		MYSQL_BIND bind;
		memset(&bind, 0, sizeof(MYSQL_BIND));
		bind.buffer_type = MYSQL_TYPE_LONGLONG;
		bind.buffer = (char*)&value;
		bind.is_null = 0;
		bind.length = 0;
		binds.push_back(bind);
		return Query(sql, binds, lengths, args...);
	}

	template<typename ...Args>
	inline bool Query(const char* sql, std::vector<MYSQL_BIND>& binds, std::vector<unsigned long>& lengths, uint64_t value, const Args& ...args) {
		lengths.push_back(0);

		MYSQL_BIND bind;
		memset(&bind, 0, sizeof(MYSQL_BIND));
		bind.buffer_type = MYSQL_TYPE_LONGLONG;
		bind.buffer = (char*)&value;
		bind.is_null = 0;
		bind.is_unsigned = true;
		bind.length = 0;
		binds.push_back(bind);
		return Query(sql, binds, lengths, args...);
	}

	template<typename ...Args>
	inline bool Query(const char* sql, std::vector<MYSQL_BIND>& binds, std::vector<unsigned long>& lengths, float value, const Args& ...args) {
		lengths.push_back(0);

		MYSQL_BIND bind;
		memset(&bind, 0, sizeof(MYSQL_BIND));
		bind.buffer_type = MYSQL_TYPE_FLOAT;
		bind.buffer = (char*)&value;
		bind.is_null = 0;
		bind.length = 0;
		binds.push_back(bind);
		return Query(sql, binds, lengths, args...);
	}

	template<typename ...Args>
	inline bool Query(const char* sql, std::vector<MYSQL_BIND>& binds, std::vector<unsigned long>& lengths, double value, const Args& ...args) {
		lengths.push_back(0);

		MYSQL_BIND bind;
		memset(&bind, 0, sizeof(MYSQL_BIND));
		bind.buffer_type = MYSQL_TYPE_DOUBLE;
		bind.buffer = (char*)&value;
		bind.is_null = 0;
		bind.length = 0;
		binds.push_back(bind);
		return Query(sql, binds, lengths, args...);
	}

	template<typename ...Args>
	inline bool Query(const char* sql, std::vector<MYSQL_BIND>& binds, std::vector<unsigned long>& lengths, bool value, const Args& ...args) {
		lengths.push_back(0);

		MYSQL_BIND bind;
		memset(&bind, 0, sizeof(MYSQL_BIND));
		bind.buffer_type = MYSQL_TYPE_BIT;
		bind.buffer = (char*)&value;
		bind.is_null = 0;
		bind.length = 0;
		binds.push_back(bind);
		return Query(sql, binds, lengths, args...);
	}

	template<typename ...Args>
	inline bool Query(const char* sql, std::vector<MYSQL_BIND>& binds, std::vector<unsigned long>& lengths, const char* value, const Args& ...args) {
		lengths.push_back(strlen(value));

		MYSQL_BIND bind;
		memset(&bind, 0, sizeof(MYSQL_BIND));
		bind.buffer_type = MYSQL_TYPE_STRING;
		bind.buffer = (char*)value;
		bind.is_null = 0;
		bind.length = &lengths[lengths.size() - 1];
		binds.push_back(bind);
		return Query(sql, binds, lengths, args...);
	}

	template<typename ...Args>
	inline bool Query(const char* sql, std::vector<MYSQL_BIND>& binds, std::vector<unsigned long>& lengths, const std::string& value, const Args& ...args) {
		lengths.push_back((unsigned long)value.size());

		MYSQL_BIND bind;
		memset(&bind, 0, sizeof(MYSQL_BIND));
		bind.buffer_type = MYSQL_TYPE_STRING;
		bind.buffer = (char*)value.c_str();
		bind.is_null = 0;
		bind.length = &lengths[lengths.size() - 1];
		binds.push_back(bind);
		return Query(sql, binds, lengths, args...);
	}

	bool Query(const char* sql, std::vector<MYSQL_BIND>& binds, std::vector<unsigned long>& lengths) {
		m_affected_rows = 0;
		m_last_error.clear();
		m_result.clear();

		auto stmt = mysql_stmt_init(&m_mysql);
		if(!stmt)
			return false;

		if(mysql_stmt_prepare(stmt, sql, (unsigned long)strlen(sql))) {
			m_last_error = mysql_stmt_error(stmt);
			mysql_stmt_close(stmt);
			return false;
		}

		unsigned int field_count = mysql_stmt_field_count(stmt);
		auto param_count = mysql_stmt_param_count(stmt);
		if(param_count != binds.size()) {
			m_last_error = "invalid parameter count";
			mysql_stmt_close(stmt);
			return false;
		}

		if(mysql_stmt_bind_param(stmt, &binds[0])) {
			m_last_error = mysql_stmt_error(stmt);
			mysql_stmt_close(stmt);
			return false;
		}

		if(mysql_stmt_execute(stmt)) {
			m_last_error = mysql_stmt_error(stmt);
			mysql_stmt_close(stmt);
			return false;
		}

		auto affected_rows = mysql_stmt_affected_rows(stmt);	//-1 for SELECT
		if(affected_rows != (uint64_t)-1)
			m_affected_rows = affected_rows;

		if(field_count > 0) {
			auto prepare_meta_result = mysql_stmt_result_metadata(stmt);
			if(!prepare_meta_result) {
				m_last_error = mysql_stmt_error(stmt);
				mysql_stmt_close(stmt);
				return false;
			}

			auto column_count = mysql_num_fields(prepare_meta_result);
			if(column_count != field_count) {
				m_last_error = "invalid field count";
				mysql_free_result(prepare_meta_result);
				mysql_stmt_close(stmt);
				return false;
			}

			MYSQL_BIND* out_binds = new MYSQL_BIND[column_count];
			char** out_buffer = new char*[column_count];
			unsigned long* out_length = new unsigned long[column_count];
			bool* out_null = new bool[column_count];
			bool* out_error = new bool[column_count];
			memset(out_binds, 0, sizeof(MYSQL_BIND) * column_count);
			for(unsigned int i = 0; i < column_count; i++) {
				out_buffer[i] = new char[4096 + 1];
				out_binds[i].buffer_type = MYSQL_TYPE_STRING;
				out_binds[i].buffer = out_buffer[i];
				out_binds[i].length = &out_length[i];
				out_binds[i].buffer_length = 4096 + 1;
				out_binds[i].is_null = &out_null[i];
				out_binds[i].error = &out_error[i];
			}

			auto fn_free = [=](){
				for(unsigned int i = 0; i < column_count; i++)
					delete[]out_buffer[i];
				delete[]out_length;
				delete[]out_buffer;
				delete[]out_binds;
				delete[]out_null;
				delete[]out_error;
			};

			if(mysql_stmt_bind_result(stmt, out_binds)) {
				m_last_error = mysql_stmt_error(stmt);
				mysql_free_result(prepare_meta_result);
				mysql_stmt_close(stmt);
				fn_free();
				return false;
			}

			if(mysql_stmt_store_result(stmt)) {
				m_last_error = mysql_stmt_error(stmt);
				mysql_free_result(prepare_meta_result);
				mysql_stmt_close(stmt);
				fn_free();
				return false;
			}

			while(!mysql_stmt_fetch(stmt)) {
				CDbRow db_row;
				for(unsigned int i = 0; i < column_count; i++) {
					std::string value = out_null[i] ? "null" : std::string(out_buffer[i], out_length[i]);
					db_row.AddColumn(value);
				}
				m_result.AddRow(db_row);
			}

			m_affected_rows = m_result.size();

			fn_free();
			mysql_free_result(prepare_meta_result);
			mysql_stmt_free_result(stmt);
		}

		return !mysql_stmt_close(stmt);
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
		std::string error = m_last_error.empty() ? mysql_error(&m_mysql) : m_last_error;
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
	std::string m_last_error;
};
