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
#include "RedisMgr.h"
using namespace std;
using json = nlohmann::json;


class ChatServer {
private:
    int listen_fd, epfd;
    ThreadPool pool;
    RedisMgr redis_mgr;
    MysqlPool db_pool; // 引入连接池
    unordered_map<int, string> fd_to_name;
    mutex user_mtx;

    bool read_n(int fd, char* buf, int len);

    void send_packet(int fd, const json& j);

public:
    ChatServer(int port);

    void run();

    // 业务逻辑：登录验证 (使用连接池)
    bool validate_db(string user, string pass);

    // 业务逻辑：存离线消息 (使用连接池)
    void save_offline_msg(string to, string from, string msg);

    // 业务逻辑：推离线消息 (使用连接池)
    void push_offline_msgs(int fd, string user);

    void handle_business(int fd, string data);

    void handle_offline(int fd);
};