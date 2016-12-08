//
// Created by Amir Razmjou on 12/5/16.
//

#ifndef TRACKDNS_THREADPOOL_H_H
#define TRACKDNS_THREADPOOL_H_H

#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>

class ThreadPool;

class Worker {
public:
    Worker(ThreadPool &s) : pool(s) { }
    void operator()();
private:
    ThreadPool &pool;
};

class ThreadPool {
public:
    ThreadPool(size_t);
    void enqueue(const std::function<void(void)> &f);
    ~ThreadPool();
private:
    friend class Worker;

    std::vector< std::thread > workers;
    // the task queue
    std::queue< std::function<void()> > tasks;

    // synchronization
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop = false;
};


#endif //TRACKDNS_THREADPOOL_H_H
