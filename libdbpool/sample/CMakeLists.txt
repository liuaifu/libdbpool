# CMakeList.txt: sample 的 CMake 项目，包括源和定义
# 此处特定于项目的逻辑。
#
cmake_minimum_required (VERSION 3.2)

PROJECT(sample)

if(WIN32)
	include_directories(../src D:/library/MySQL-5.7/include)
	link_directories(D:/library/MySQL-5.7/lib)
else()
	include_directories(../src)
endif(WIN32)

# 将源添加到此项目的可执行文件。
add_executable (${PROJECT_NAME} "sample.cpp")

# TODO: 如有需要，请添加测试并安装目标。
if(WIN32)
	TARGET_LINK_LIBRARIES(${PROJECT_NAME} libmysql.lib)
else()
	TARGET_LINK_LIBRARIES(${PROJECT_NAME} -static -static-libgcc -static-libstdc++ mysqlclient pthread z dl)
endif(WIN32)
