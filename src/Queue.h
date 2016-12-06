//
// Created by Amir Razmjou on 12/4/16.
//

#ifndef TRACKDNS_QUEUE_H
#define TRACKDNS_QUEUE_H
#include <queue>
#include <thread>
#include <mutex>
#include <vector>
#include <condition_variable>

template <typename T>
class Queue
{
public:
    std::vector<T> popAll() {
        std::unique_lock<std::mutex> mlock(mutex_);
        std::vector<T> vec;
        while (!queue_.empty())
        {
            auto val = queue_.front();
            vec.push_back(val);
            queue_.pop();
        }
        return vec;
    }

    size_t size() {
        queue_.size();
    }

    T pop()
    {
        std::unique_lock<std::mutex> mlock(mutex_);
        while (queue_.empty())
        {
            cond_.wait(mlock);
        }
        auto val = queue_.front();
        queue_.pop();
        return val;
    }

    void pop(T& item)
    {
        std::unique_lock<std::mutex> mlock(mutex_);
        while (queue_.empty())
        {
            cond_.wait(mlock);
        }
        item = queue_.front();
        queue_.pop();
    }

    void push(const T& item)
    {
        std::unique_lock<std::mutex> mlock(mutex_);
        queue_.push(item);
        mlock.unlock();
        cond_.notify_one();
    }

    Queue()=default;
    Queue(const Queue&) = delete;            // disable copying
    Queue& operator=(const Queue&) = delete; // disable assignment

private:
    std::queue<T> queue_;
    std::mutex mutex_;
    std::condition_variable cond_;
};

#endif //TRACKDNS_QUEUE_H
