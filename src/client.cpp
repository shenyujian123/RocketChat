#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::json;

// --- 工具函数：确保读取 N 个字节 ---
bool read_n(int fd, char* buf, int len) {
    int total = 0;
    while (total < len) {
        int n = recv(fd, buf + total, len - total, 0);
        if (n <= 0) return false;
        total += n;
    }
    return true;
}

// --- 工具函数：按照协议发送 [4字节头][JSON体] ---
void send_packet(int fd, const json& j) {
    string s = j.dump();
    int len = s.size();
    int net_len = htonl(len); // 转为网络字节序
    send(fd, (char*)&net_len, 4, 0);
    send(fd, s.c_str(), len, 0);
}

// --- 线程函数：专门负责接收并显示消息 ---
void receive_worker(int sock) {
    while (true) {
        int net_len = 0;
        if (!read_n(sock, (char*)&net_len, 4)) {
            cout << "\n【系统】与服务器断开连接。" << endl;
            exit(0);
        }
        int body_len = ntohl(net_len);

        vector<char> buf(body_len + 1, 0);
        if (!read_n(sock, buf.data(), body_len)) break;
        
        try {
            auto j = json::parse(buf.data());
            if (j.count("type") && j["type"] == "msg") {
                cout << "\n【私聊】" << j["from"] << ": " << j["content"] << endl;
                cout << "发送给(格式 名字:内容): " << flush;
            } else if (j.count("type") && j["type"] == "system") {
                cout << "\n【系统消息】: " << j["content"] << endl;
                cout << "发送给(格式 名字:内容): " << flush;
            } else {
                cout << "\n【服务器回执】: " << j.dump() << endl;
                cout << "发送给(格式 名字:内容): " << flush;
            }
        } catch (...) {
            cout << "\n【错误】收到无法解析的数据。" << endl;
        }
    }
}

int main() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr = {AF_INET, htons(8888), {inet_addr("127.0.0.1")}};

    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        cerr << "无法连接到本地服务器，请确保服务器已启动！" << endl;
        return -1;
    }

    // 1. 登录逻辑
    string username;
    cout << "请输入用户名 (jack/rose): ";
    cin >> username;
    
    json login_j = {{"type", "login"}, {"user", username}, {"pass", "123456"}};
    send_packet(sock, login_j);

    // 2. 启动接收线程
    thread(receive_worker, sock).detach();

    // 3. 发送逻辑
    cout << "--- 登录请求已发送 ---" << endl;
    cout << "发送格式: 名字:内容 (例如 rose:hello)，输入 exit 退出" << endl;

    string input;
    getline(cin, input); // 吃掉多余的回车
    while (true) {
        cout << "发送给(格式 名字:内容): ";
        getline(cin, input);
        if (input == "exit") break;

        size_t pos = input.find(':');
        if (pos != string::npos) {
            string to = input.substr(0, pos);
            string content = input.substr(pos + 1);
            json chat_j = {{"type", "chat_private"}, {"to", to}, {"content", content}};
            send_packet(sock, chat_j);
        } else {
            cout << "格式错误，请使用 '名字:内容'" << endl;
        }
    }

    close(sock);
    return 0;
}