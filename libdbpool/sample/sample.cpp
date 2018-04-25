// sample.cpp: 定义应用程序的入口点。
//

#include <iostream>
#include <libdbpool/DbPool.h>

int main()
{
	CDbPool pool("localhost", "root", "123456", "test", 1);

	//测试最大连接数
	assert(pool.GetIdleCount()==0);		//连接是动态创建的，初始时没有创建任何连接
	assert(pool.GetUsingCount()==0);
	auto cn1 = pool.GetDbConnection();
	assert(pool.GetIdleCount()==0);
	assert(pool.GetUsingCount()==1);
	auto cn2 = pool.GetDbConnection();
	assert(pool.GetIdleCount()==0);
	assert(pool.GetUsingCount()==1);
	assert(cn1!=nullptr);
	assert(cn2==nullptr);
	pool.FreeDbConnection(cn1);
	assert(pool.GetIdleCount()==1);
	assert(pool.GetUsingCount()==0);
	pool.FreeDbConnection(cn2);
	assert(pool.GetIdleCount()==1);
	assert(pool.GetUsingCount()==0);

	//创建库表
	int n = pool.ExecuteNonQuery("create database If Not Exists test Character Set UTF8");
	assert(n != -1);
	n = pool.ExecuteNonQuery("create table If Not Exists test_table(id int, data varchar(50))");
	assert(n == 0);
	n = pool.ExecuteNonQuery("delete from test_table");
	assert(n != -1);

	//插入数据
	n = pool.ExecuteNonQuery("insert into test_table(id,data) values(1,'hello')");
	assert(n == 1);
	n = pool.ExecuteNonQuery("insert into test_table(id,data) values(2,'world')");
	assert(n == 1);

	//测试查询
	auto cn = pool.GetDbConnection();
	assert((bool)cn);
	bool ret = cn->Query("select id,data from test_table order by id asc");
	assert(ret);
	auto rows = cn->GetResult();
	assert(rows.size() == 2);
	assert(rows[0].size() == 2);
	assert(rows[1].size() == 2);
	assert(std::string(rows[0][0]) == "1");
	assert(std::string(rows[0][1]) == "hello");
	assert(std::string(rows[1][0]) == "2");
	assert(std::string(rows[1][1]) == "world");
	pool.FreeDbConnection(cn);


	//清理
	n = pool.ExecuteNonQuery("delete from test_table");
	assert(n == 2);
	n = pool.ExecuteNonQuery("drop table test_table");
	assert(n == 0);

	std::cout << "pass.";

	return 0;
}
