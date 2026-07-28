#include "ThreadSafeQueue.h"
#include <thread>
int main(){
    ThreadSafeQueue<int> tsq(10);
    std::thread producer([&tsq]{
        for(int i=1;i<=10;i++){
            tsq.push(i);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });
    std::thread consumer([&tsq]{
        for(int i=1;i<=10;i++){
            int val = tsq.pop();
            std::this_thread::sleep_for(std::chrono::milliseconds(150));
        }
    });
    producer.join();
    consumer.join();
    return 0;

}
//RUN: g++ -std=c++17 ThreadSafeQueue.cpp -o ThreadSafeQueue -lpthread
