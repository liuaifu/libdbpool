// sample.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "DbPool.h"

int main()
{
	CDbPool pool("localhost", "root", "123", "test");

	for (int i = 0;i < 10000;i++) {
		int n = pool.ExecuteNonQuery("delete from data");
		printf("%d: 删除了%d行\n", i, n);
	}

	auto pConnection = pool.GetDbConnection();
	bool ret = false;
	//ret = pConnection->Query("insert into data(id,data1,data2) values(1,100,200)");
	//assert(ret);
	//ret = pConnection->Query("insert into data(id,data1,data2) values(1,200,400)");
	//assert(ret);
	//ret = pConnection->Query("insert into data(id,data1,data2) values(1,300,600)");
	//assert(ret);
	//ret = pConnection->Query("insert into data(id,data1,data2) values(1,-100,NULL)");
	//assert(ret);
	ret = pConnection->Query("select id,data1,data2 from data");
	if (ret) {
		auto db_result = pConnection->GetResult();
		size_t n = db_result.size();
		for (size_t i = 0;i < n;i++)
		{
			auto row = db_result[i];
			for (size_t j = 0;j < row.size();j++)
			{
				auto col = row[j];
				printf("%s ", col);
			}
			printf("\n");
		}
	}
	pool.FreeDbConnection(pConnection);
    return 0;
}

