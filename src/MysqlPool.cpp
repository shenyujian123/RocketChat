#include "MysqlPool.h"

MysqlPool:: MysqlPool(string host, string user, string pass, string db, int port, int size) : pool_size(size)
{
    for (int i = 0; i < pool_size; ++i) {
        MYSQL* conn = mysql_init(NULL);
        if (mysql_real_connect(conn, host.c_str(), user.c_str(), pass.c_str(), db.c_str(), port, NULL, 0)) {
            conn_queue.push(conn);
        } else {
            cerr << "MySQL Pool Init Error: " << mysql_error(conn) << endl;
        }
    }
}
MYSQL* MysqlPool:: getConnection()
{
    unique_lock<mutex> lock(mtx);
    cv.wait(lock, [this]{ return !conn_queue.empty(); });
    MYSQL* conn = conn_queue.front();
    conn_queue.pop();
    return conn;
}
void MysqlPool:: releaseConnection(MYSQL* conn) {
    unique_lock<mutex> lock(mtx);
    conn_queue.push(conn);
    cv.notify_one();
}
MysqlPool:: ~MysqlPool() {
    while(!conn_queue.empty()){
        mysql_close(conn_queue.front());
        conn_queue.pop();
    }
}