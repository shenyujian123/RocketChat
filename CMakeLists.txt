# 1. 基础环境配置
cmake_minimum_required(VERSION 3.10)
project(RocketChat VERSION 6.0)

# 强制使用 C++17 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 2. 查找第三方依赖库
# 查找线程库 (pthread)
find_package(Threads REQUIRED)

# 查找 MySQL 库
find_library(MYSQL_LIB NAMES mysqlclient REQUIRED)

# 查找 hiredis 库
find_library(HIREDIS_LIB NAMES hiredis REQUIRED)

# ========================================================
# 3. 构建服务端可执行文件：RocketServer
# ========================================================

# 定义服务端所有的源文件（注意路径前缀 src/）
set(SERVER_SOURCES
    src/main.cpp
    src/ChatServer.cpp
    src/MysqlPool.cpp
    src/RedisMgr.cpp
    src/ConnectionGuard.cpp
    src/ThreadPool.cpp
)

# 生成可执行文件
add_executable(RocketServer ${SERVER_SOURCES})

# 【核心修复】：精准挂载头文件搜索路径
# PRIVATE 确保这些路径只对 RocketServer 有效，防止污染其他目标
target_include_directories(RocketServer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include     # 我们自己的 .h 文件夹
    /usr/include/mysql                      # MySQL 系统头文件路径
    /usr/include/hiredis                    # Redis 系统头文件路径
)

# 链接对应的二进制库
target_link_libraries(RocketServer
    ${MYSQL_LIB}
    ${HIREDIS_LIB}
    Threads::Threads
)

# ========================================================
# 4. 构建客户端可执行文件：chat_client
# ========================================================

# 客户端只需要 client.cpp 
add_executable(chat_client src/client.cpp)

# 客户端也需要包含 include 文件夹（为了使用协议相关的定义）
target_include_directories(chat_client PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 客户端链接线程库即可
target_link_libraries(chat_client
    Threads::Threads
)

# ========================================================
# 5. 编译信息打印 (可选，方便排查)
# ========================================================
message(STATUS "-----------------------------------------")
message(STATUS "Project Path: ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "MySQL Lib Found: ${MYSQL_LIB}")
message(STATUS "Hiredis Lib Found: ${HIREDIS_LIB}")
message(STATUS "-----------------------------------------")