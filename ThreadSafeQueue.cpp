//Implement a bounded thread safe queue.
// C++ program to illustrate the use of shared_mutex
#include <shared_mutex>
#include <mutex>
#include <queue>
#include <thread>
#include <iostream>
#include<condition_variable>
using namespace std;

// creating a shared_mutex object
class ThreadSafeQueue {
    private :
        queue<int> q;
        mutex mtx;
        int capacity;
        condition_variable cv_when_full, cv_when_empty;

    public:
        ThreadSafeQueue(size_t cap):capacity(cap){}

    void push(int el){
        unique_lock<mutex> lg(mtx);
        cv_when_full.wait(lg,[this]{return q.size()<capacity;});
        q.push(el);
                    std::cout<<"Pushed "<<el<<endl;

        lg.unlock();
        cv_when_empty.notify_one();
    }

    int pop(){
        unique_lock<mutex> lg(mtx);
        cv_when_empty.wait(lg,[this]{return !q.empty();});
        int val = q.front();
        q.pop();
        std::cout<<"Popped "<<val<<endl;
        lg.unlock();
        cv_when_full.notify_one();
        return val;
    }
    
    
};
int main(){
    ThreadSafeQueue tsq(10);
    thread producer([&tsq]{
        for(int i=1;i<=10;i++){
            tsq.push(i);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });
    thread consumer([&tsq]{
        for(int i=1;i<=10;i++){
            int val = tsq.pop();
            std::cout<<"Popped "<<val<<endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(150));
        }
    });
    producer.join();
    consumer.join();
    return 0;

}
//RUN: g++ -std=c++17 ThreadSafeQueue.cpp -o ThreadSafeQueue -lpthread
