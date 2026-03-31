#include "ChatServer.h"

ChatServer :: ChatServer(int port) : pool(8), db_pool("localhost", "dev", "123456", "demo_db", 3306, 10)
{
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1; setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    sockaddr_in addr = {AF_INET, htons(port), {INADDR_ANY}};
    bind(listen_fd, (struct sockaddr *)&addr, sizeof(addr));
    listen(listen_fd, 100);
    epfd = epoll_create1(0);
    struct epoll_event ev = {EPOLLIN, {.fd = listen_fd}};
    epoll_ctl(epfd, EPOLL_CTL_ADD, listen_fd, &ev);
}
bool ChatServer :: read_n(int fd, char* buf, int len)
{
    int total = 0;
    while (total < len) {
        int n = recv(fd, buf + total, len - total, 0);
        if (n <= 0) return false;
        total += n;
    }
    return true;
}
void ChatServer :: send_packet(int fd, const json& j) 
{
    string s = j.dump();
    int net_len = htonl(s.size());
    send(fd, (char*)&net_len, 4, 0);
    send(fd, s.c_str(), s.size(), 0);
}
void ChatServer :: run() 
{
    struct epoll_event events[1024];
    while (true) {
        int n = epoll_wait(epfd, events, 1024, -1);
        for (int i = 0; i < n; i++) {
            int fd = events[i].data.fd;
            if (fd == listen_fd) {
                int c_fd = accept(listen_fd, NULL, NULL);
                struct epoll_event ev = {EPOLLIN, {.fd = c_fd}};
                epoll_ctl(epfd, EPOLL_CTL_ADD, c_fd, &ev);
            } else {
                int net_len = 0;
                if (!read_n(fd, (char*)&net_len, 4)) { handle_offline(fd); continue; }
                int body_len = ntohl(net_len);
                vector<char> buf(body_len + 1, 0);
                if (!read_n(fd, buf.data(), body_len)) { handle_offline(fd); continue; }
                string data(buf.data());
                pool.enqueue([this, fd, data] { this->handle_business(fd, data); });
            }
        }
    }
}
bool ChatServer :: validate_db(string user, string pass) {
    MYSQL* conn = nullptr;
    ConnectionGuard guard(&conn, &db_pool);
    
    string sql = "SELECT * FROM chat_users WHERE username='" + user + "' AND password='" + pass + "'";
    mysql_query(conn, sql.c_str());
    
    MYSQL_RES *res = mysql_store_result(conn);
    // --- 【防崩检查】 ---
    if (res == nullptr) return false;

    int rows = mysql_num_rows(res);
    mysql_free_result(res);
    return rows > 0;
}
void ChatServer :: save_offline_msg(string to, string from, string msg) 
{
    MYSQL* conn = nullptr;
    ConnectionGuard guard(&conn, &db_pool);
    string content = "[" + from + "的留言]: " + msg;
    string sql = "INSERT INTO offline_messages (to_user, message) VALUES ('" + to + "', '" + content + "')";
    mysql_query(conn, sql.c_str());
}
void ChatServer :: push_offline_msgs(int fd, string user) {
    MYSQL* conn = nullptr;
    ConnectionGuard guard(&conn, &db_pool);
    
    string sql = "SELECT message FROM offline_messages WHERE to_user='" + user + "'";
    if (mysql_query(conn, sql.c_str()) != 0) {
        cerr << "查询离线消息失败: " << mysql_error(conn) << endl;
        return; 
    }

    MYSQL_RES *res = mysql_store_result(conn);
    // --- 【防崩检查 1】 ---
    if (res == nullptr) return; 

    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res))) {
        // --- 【防崩检查 2】 ---
        if (row && row[0]) {
            send_packet(fd, {{"type","system"}, {"content", row[0]}});
        }
    }

    mysql_free_result(res);
    
    // 执行删除逻辑
    string del_sql = "DELETE FROM offline_messages WHERE to_user='" + user + "'";
    mysql_query(conn, del_sql.c_str());
}
void ChatServer :: handle_business(int fd, string data) {
    try {
        auto j = json::parse(data);
        string type = j["type"];
        if (type == "login") {
            if (validate_db(j["user"], j["pass"])) {
                { lock_guard<mutex> lock(user_mtx); fd_to_name[fd] = j["user"]; }
                redis_mgr.set_online(j["user"], fd);
                send_packet(fd, {{"status","ok"}});
                push_offline_msgs(fd, j["user"]);
            } else send_packet(fd, {{"status","err"}});
        } 
        else if (type == "chat_private") {
            string to = j["to"], content = j["content"], from;
            { lock_guard<mutex> lock(user_mtx); from = fd_to_name[fd]; }
            int target_fd = redis_mgr.get_fd(to);
            if (target_fd != -1) {
                send_packet(target_fd, {{"type","msg"}, {"from", from}, {"content", content}});
            } else {
                save_offline_msg(to, from, content);
            }
        }
    } catch (...) {}
}
void ChatServer :: handle_offline(int fd) 
{
    lock_guard<mutex> lock(user_mtx);
    if (fd_to_name.count(fd)) {
        redis_mgr.set_offline(fd_to_name[fd]);
        fd_to_name.erase(fd);
    }
    epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
    close(fd);
}