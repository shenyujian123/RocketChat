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
#include "ChatServer.h"
using namespace std;
using json = nlohmann::json;

int main() {
    ChatServer server(8888);
    cout << "RocketChat 6.0 (终极连接池版) 运行中..." << endl;
    server.run();
    return 0;
}