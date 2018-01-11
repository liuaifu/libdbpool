#include "libdbpool/DbConnection.h"
#include <assert.h>


CDbConnection::CDbConnection(std::string host, std::string user, std::string passwd, std::string database, unsigned int port, std::string charset_name)
	:m_host(host), m_user(user), m_passwd(passwd), m_database(database), m_port(port), m_affected_rows(0)
{
	MYSQL* mysql = mysql_init(&m_mysql);
	assert(mysql!=NULL);

	mysql_options(mysql, MYSQL_SET_CHARSET_NAME, charset_name.c_str());
	//mysql_options(&m_mysql, MYSQL_OPT_COMPRESS, 0);
	my_bool value = 1;
	mysql_options(&m_mysql, MYSQL_OPT_RECONNECT, &value);
	bool ret = Connect();
	const char *error = mysql_error(mysql);
	assert(ret);
}

CDbConnection::~CDbConnection()
{
	mysql_close(&m_mysql);
}

bool CDbConnection::Connect()
{
	return NULL != mysql_real_connect(&m_mysql, m_host.c_str(), m_user.c_str(), m_passwd.c_str(), m_database.c_str(), m_port, NULL, 0/*CLIENT_MULTI_RESULTS*/);
}

bool CDbConnection::Query(const char* sql)
{
	m_affected_rows = 0;
	m_result.clear();

	if (mysql_ping(&m_mysql))
		return false;

	if (mysql_query(&m_mysql, sql))
		return false;
	
	m_affected_rows = mysql_affected_rows(&m_mysql);

	MYSQL_RES *pRes = mysql_store_result(&m_mysql);
	if (pRes == NULL)
		return true;	//∑«≤È—Ø

	while(MYSQL_ROW row = mysql_fetch_row(pRes))
	{
		CDbRow db_row;
		for (size_t i = 0;i < mysql_num_fields(pRes);i++)
		{
			db_row.AddColumn(row[i] ? row[i] : "null");
		}
		m_result.AddRow(db_row);
	}

	mysql_free_result(pRes);
	return true;
}
