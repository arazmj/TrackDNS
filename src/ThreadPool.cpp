//
// Created by Amir Razmjou on 12/7/16.
//

#include "ThreadPool.h"

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

void ThreadPool::enqueue(const std::function<void(void)> &f)
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