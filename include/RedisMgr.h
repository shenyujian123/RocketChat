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
#include "ConnectionGuard.h"
#include "ThreadPool.h"
using namespace std;
using json = nlohmann::json;
class RedisMgr {
private:
    redisContext *context;
    mutex mtx;
public:
    RedisMgr();
    void set_online(string user, int fd);
    void set_offline(string user);
    int get_fd(string user);
    ~RedisMgr();
};