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

// the actual thread pool
class ThreadPool {
public:
    ThreadPool(size_t);
    template<class F>
    void enqueue(F f);
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

void Worker::operator()()
{
    while(true)
    {
        std::unique_lock<std::mutex> lock(pool.queue_mutex);
        while(!pool.stop && pool.tasks.empty())
            pool.condition.wait(lock);
        if(pool.stop && pool.tasks.empty())
            return;
        std::function<void()> task(pool.tasks.front());
        pool.tasks.pop();
        lock.unlock();
        task();
    }
}

ThreadPool::ThreadPool(size_t threads)
{
    for(size_t i = 0;i<threads;++i)
        workers.push_back(std::thread(Worker(*this)));
}

template<class F>
void ThreadPool::enqueue(F f)
{
    if(stop)
        throw std::runtime_error("enqueue on stopped ThreadPool");

    auto task = std::make_shared<std::packaged_task<void()> >(f);
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        tasks.push([task](){ (*task)(); });
    }
    condition.notify_one();
}

ThreadPool::~ThreadPool()
{
    stop = true;
    condition.notify_all();
    for(size_t i = 0;i<workers.size();++i)
        workers[i].join();
}

#endif //TRACKDNS_THREADPOOL_H_H
