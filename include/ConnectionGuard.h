#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <cstring>
#include <mysql/mysql.h>
#include <hiredis/hiredis.h>
#include <nlohmann/json.hpp>
#include "MysqlPool.h"
using namespace std;
class ConnectionGuard {
    MYSQL* conn; MysqlPool* pool;
public:
    ConnectionGuard(MYSQL** c, MysqlPool* p);
    ~ConnectionGuard();
};