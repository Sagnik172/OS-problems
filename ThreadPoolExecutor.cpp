#include "ThreadSafeQueue.h"
#include <vector>
#include <thread>
#include <functional>
#include <vector>
#include <queue>
#include <mutex>
#include <future> // Required for std::future
#include <condition_variable>

class ThreadPoolExecutor {

    private:
    std::queue<std::function<void()>> tasks;
    std::vector<std::thread> threads;
    size_t numThreads;
    std::mutex mtx;
    std::condition_variable cv;
    bool stop;

    public:
    explicit ThreadPoolExecutor(size_t num):numThreads(num),stop(false){

        for(size_t i=0;i<numThreads;i++){
            threads.emplace_back([this]{
                std::function<void()> task;
                while (true)
                {
                    std::unique_lock<std::mutex> lock(mtx);
                    cv.wait(lock,[this]{return this->stop || !this->tasks.empty();});
                    if(stop){
                        std::cout<<"Thread killed"<<std::endl;
                        return;
                    }
                    task = tasks.front();
                    tasks.pop();
                    lock.unlock();
                    task();
                    /* code */
                }
                
            });
        }
    }

    // The submit method should be a member function template, declared directly within the class.
    // The return type should correctly deduce the result of calling f with args.
    template<class F, class... Args>
    auto submit(F&& f, Args&&... args) -> std::future<decltype(f(args...))> {
        using return_type = decltype(f(args...));
        auto task = std::make_shared<std::packaged_task<return_type()>>(std::bind(std::forward<F>(f),std::forward<Args>(args)...));
        std::future<return_type> res = task->get_future();
        std::unique_lock<std::mutex> lock(mtx);
        if(stop){
            throw std::runtime_error("ThreadPoolExecutor has been stopped. Cannot submit new tasks."); 
        }
        tasks.push([task]{
            (*task)();
        });
        lock.unlock();
        cv.notify_one();
        return res;
    }
    ~ThreadPoolExecutor(){
        {
            std::unique_lock<std::mutex> lock(mtx);
            stop = true;
        }
        cv.notify_all();
        for(std::thread &worker: threads){
            if(worker.joinable()){
                worker.join();
            }
        }
    }
};
int main(){
    ThreadPoolExecutor executor(2);
    auto future1 = executor.submit([](int a, int b,int c){return a+b+c;}, 5, 10, 15);
    auto future2 = executor.submit([](std::string a, std::string b){return a+b;}, "Hello, ", "World!");
    std::cout<<"Sum: "<<future1.get()<<"\n";
    std::cout<<"Concatenation: "<<future2.get()<<"\n";
    return 0;
}
//RUN: g++ -std=c++17 ThreadPoolExecutor.cpp -o ThreadPoolExecutor -lpthread