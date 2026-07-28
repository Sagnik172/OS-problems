//Implement a bounded thread safe queue.
#pragma once
#include <queue>
#include <mutex>
#include <thread>
#include <iostream>
#include <condition_variable>

template <typename T>
class ThreadSafeQueue {
private :
    std::queue<T> q;
    std::mutex mtx;
    size_t capacity;
    std::condition_variable cv_when_full, cv_when_empty;

public:
    ThreadSafeQueue(size_t cap):capacity(cap){}

    void push(T el){
        std::unique_lock<std::mutex> lg(mtx);
        cv_when_full.wait(lg,[this]{return q.size()<capacity;});
        q.push(std::move(el));

        lg.unlock();
        cv_when_empty.notify_one();
    }

    T pop(){
        std::unique_lock<std::mutex> lg(mtx);
        cv_when_empty.wait(lg,[this]{return !q.empty();});
        T val = std::move(q.front());
        q.pop();
        lg.unlock();
        cv_when_full.notify_one();
        return val;
    }
};
