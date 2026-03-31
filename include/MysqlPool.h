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

using namespace std;
using json = nlohmann::json;
class MysqlPool {
private:
    queue<MYSQL*> conn_queue;
    mutex mtx;
    condition_variable cv;
    int pool_size;

public:
    MysqlPool(string host, string user, string pass, string db, int port, int size);
    MYSQL* getConnection();

    void releaseConnection(MYSQL* conn);

    ~MysqlPool();
};