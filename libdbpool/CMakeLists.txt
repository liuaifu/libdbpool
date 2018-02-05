CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

AUX_SOURCE_DIRECTORY(./src DIR_SRCS)

#添加动态库
ADD_LIBRARY(dbpool SHARED ${DIR_SRCS})
#添加静态库
ADD_LIBRARY(dbpool_static STATIC ${DIR_SRCS})

include_directories(./include ../../lib/boost/include)

#生成动态库的版本号
SET_TARGET_PROPERTIES(dbpool PROPERTIES VERSION 1.0 SOVERSION 1)
#将静态库重新命名
SET_TARGET_PROPERTIES(dbpool_static PROPERTIES OUTPUT_NAME "dbpool")
  
#安装静态库和动态库  
INSTALL(TARGETS dbpool dbpool_static
	    LIBRARY DESTINATION lib
	        ARCHIVE DESTINATION lib)
#安装hello.h
INSTALL(DIRECTORY include/libdbpool DESTINATION include)
