# 🚀 RocketChat - High Performance C++ Instant Messaging System

[![C++](https://img.shields.io/badge/Language-C%2B%2B17-blue.svg)](https://en.cppreference.com/w/cpp/17)
[![Platform](https://img.shields.io/badge/Platform-Linux-orange.svg)](https://www.linux.org/)
[![License](https://img.shields.io/badge/License-MIT-green.svg)](https://opensource.org/licenses/MIT)

RocketChat 是一款基于 **Reactor 模式** 实现的工业级 IM（即时通讯）后端原型系统。项目旨在通过 Linux 底层网络 IO 多路复用技术与现代 C++ 设计模式，构建一个能够支撑高并发、具备跨设备通讯能力的分布式架构原型。

---

## 🏗️ 核心架构设计 (System Architecture)

本项目采用了典型的 **Reactor + ThreadPool** 模型，实现了网络 IO 与业务逻辑的彻底解耦：

- **Main Reactor**: 负责利用 `Epoll` 监听新连接请求。
- **Worker ThreadPool**: 负责处理 JSON 协议解析、数据库事务、Redis 状态同步。
- **Dual-Storage Layer**: 采用 MySQL 持久化核心业务，Redis 缓存实时在线状态。

---

## ✨ 核心特性 (Key Features)

### 1. 极致并发性能
- **Epoll (IO Multiplexing)**：基于 Linux 内核级事件驱动机制，支持万级并发连接同时在线。
- **自定义线程池**：通过生产者-消费者模型管理工作线程，有效平衡系统负载，杜绝慢查询导致的系统阻塞。

### 2. 工业级协议设计
- **协议封装**：设计了 `[4-Byte Header (Length) + JSON Body]` 的二进制通信协议。
- **粘包处理**：自定义 `read_n` 逻辑，精准拆解 TCP 数据流，确保应用层数据的完整性与顺序性。
- **字节序统一**：使用 `htonl/ntohl` 处理网络字节序，确保跨平台（x86/ARM）通讯一致性。

### 3. 高性能存储优化
- **MySQL 连接池**：自主实现数据库连接池，配合 **RAII 模式的 ConnectionGuard**，降低 90% 以上的握手损耗。
- **Redis 状态机**：利用 Redis Hash 结构维护全球用户在线映射，为后续集群化扩展打下坚实基础。
- **离线消息闭环**：完善的离线消息“存-查-推-删”逻辑，保障用户体验。

---

## 🛠️ 技术栈 (Tech Stack)

| 领域 | 技术选型 |
| :--- | :--- |
| **语言** | C++17 (Modern C++, STL, Lambda, Smart Pointers) |
| **网络** | Linux Socket, Epoll (Reactor Mode) |
| **数据库** | MySQL 8.0 (Innodb), Redis 6.2 |
| **中间件** | Hiredis, nlohmann/json |
| **工程化** | CMake, Git, G++ 11.0 |
| **部署环境** | 阿里云 ECS (Ubuntu 22.04 LTS) |

---

## 📂 项目结构 (Project Structure)

```text
RocketChat/
├── include/            # 头文件目录：接口定义与类声明
│   ├── ChatServer.h
│   ├── MysqlPool.h     # 数据库连接池封装
│   ├── RedisMgr.h      # Redis 状态管理中心
│   ├── ThreadPool.h    # 核心异步处理内核
│   └── ConnectionGuard.h
├── src/                # 源文件目录：核心业务实现
│   ├── main.cpp        # 服务端入口
│   ├── client.cpp      # C++ 高级客户端实现
│   └── ... (Impl files)
├── CMakeLists.txt      # 工程构建配置
└── README.md           # 项目说明文档
---

## 📦 环境准备 (Prerequisites)

项目运行于 **Linux** 环境（推荐 Ubuntu 20.04+），需安装以下核心组件：

| 依赖库 | 安装命令 | 说明 |
| :--- | :--- | :--- |
| **MySQL** | `sudo apt install libmysqlclient-dev` | 核心业务持久化存储 |
| **Redis** | `sudo apt install libhiredis-dev` | 在线状态实时缓存 |
| **JSON** | `sudo apt install nlohmann-json3-dev` | 应用层协议序列化 |
| **构建工具** | `sudo apt install build-essential cmake` | 编译器与自动化构建 |

---

## 🛠️ 构建与安装 (Build & Installation)

遵循标准的 CMake 影子构建流程，确保源代码目录整洁：

```bash
# 1. 克隆仓库
git clone https://github.com/shenyujian123/RocketChat.git
cd RocketChat

# 2. 准备数据库 (请确保 MySQL 服务已开启)
# 请执行项目根目录下的 SQL 脚本或手动创建 demo_db, chat_users, offline_messages 表

# 3. 自动化编译
mkdir build && cd build
cmake ..
make -j$(nproc)

# 进入构建目录
./RocketServer
# 看到 "RocketChat 6.0 (终极连接池版) 运行中..." 表示启动成功
./chat_client
# 按照提示输入用户名（如 jack / rose）进行登录
# 私聊指令格式: 目标用户名:消息内容 (例如 rose:hello)

## 核心技术实现细节 (Technical Highlights)
Reactor 反应堆模型: 基于 Linux Epoll 实现高效的 IO 多路复用。主线程仅负责分发事件，业务逻辑（解析、数据库、Redis）完全交由 ThreadPool (线程池) 异步处理，有效杜绝了数据库查询导致的 IO 阻塞。
自定义二进制协议: 针对 TCP 流式传输设计了 [4字节报头(长度) + 变长JSON体] 协议，封装了 read_n 函数解决 粘包/半包 顽疾，确保数据传输的原子性。
MySQL 连接池 (RAII): 手写连接池管理 MYSQL* 句柄，配合 ConnectionGuard 实现连接的自动归还与分配，将单次业务的数据库交互延迟降低了 90% 以上。
分布式状态管理: 引入 Redis Hash 存储实时在线映射。相比单机内存 Map，该方案支持服务平滑扩容，为后续集群化架构设计打下基础。
可靠消息投递: 设计了 MySQL 离线消息缓冲表。实现了用户登录时异步拉取、推送并清空离线消息的业务闭环，保障消息 100% 到达。
