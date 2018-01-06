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
	// ��������
	std::vector<CDbConnection*> m_vtIdleConnections;

	// ���ڱ�ʹ�õ�����
	std::vector<CDbConnection*> m_vtBusyConnections;

	// ��Դ��
	boost::mutex m_mutex;

	// ���������
	size_t m_maxConnection;

	//���ݿ���Ϣ
	std::string m_host;
	std::string m_user;
	std::string m_passwd;
	unsigned int m_port;
	std::string m_database;

public:
	// ��ȡһ�����е����ӣ��������ʹ�õ����Ӵﵽ���ֵ������ֱ���п��е������ͷ�
	CDbConnection* GetDbConnection();

	// ��ȡһ�����е����ӣ��������ʹ�õ����Ӵﵽ���ֵ������ֱ���п��е������ͷ�
	void FreeDbConnection(CDbConnection* pConnection);

	/**
	* ִ�зǲ�ѯ���ɹ�����Ӱ���������ʧ�ܷ���-1
	*/
	int ExecuteNonQuery(const char* sql);
};