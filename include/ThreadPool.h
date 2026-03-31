#pragma once
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
using namespace std;
class ThreadPool {
public:
    ThreadPool(size_t threads);
    void enqueue(function<void()> f);
    ~ThreadPool();

private:
    vector<thread> workers;
    queue<function<void()>> tasks;
    mutex queue_mutex;
    condition_variable condition;
    bool stop;
};