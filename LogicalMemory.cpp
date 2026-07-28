#include <string>
#include <thread>
#include <chrono>
#include <list>
#include <map>
#include <iostream>
#include <unordered_map>
#include <mutex>
#include <vector>


using PageId =std::string;

class Page{
    public:
    PageId id;
    std::string data;
};

class PhysicalMemory{

    public:
    static Page getPage(PageId id){
        // Simulate fetching a page from physical memory
        Page page;
        page.id = id;
        page.data = "Data for page " + id;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // Simulate delay
        return page;
    }

};

class LogicalMemory{

private:
    std::list<PageId> pageIdList;
    size_t cacheSize;
    using pageListIterator = std::list<PageId>::iterator;
    std::unordered_map<PageId,std::pair<Page,pageListIterator>> idPageMapping;
    std::mutex mtx;
    
public:
    LogicalMemory(size_t size):cacheSize(size){}
    Page getPage(PageId id){
        {
            std::lock_guard<std::mutex> lg(mtx);
            auto it = idPageMapping.find(id);
            if(it!=idPageMapping.end()){
                std::cout << "[HIT] Page " << id << " found in logical memory.\n";
                //dest pos,source list,iterator
                pageIdList.splice(pageIdList.begin(),pageIdList,it->second.second);
                return it->second.first;
            }

        }
        
        //check physical memory
        std::cout << "[MISS] Page " << id << " not in logical memory.\n";
        Page physicalMemPage = PhysicalMemory::getPage(id);
        {
            std::lock_guard<std::mutex> lg(mtx);
                 if(idPageMapping.count(id)){
            auto it = idPageMapping[id];
            //dest pos,source list,iterator
            std::cout << "[DOUBLE-CHECK HIT] Thread fetched " << id << " but it was just added by another thread!\n";
            pageIdList.splice(pageIdList.begin(),pageIdList,it.second);
            return it.first;
                }
        if(pageIdList.size()>=cacheSize){
            PageId removedId = pageIdList.back();
            std::cout<< "[EVICT] Memory full. Evicting page " << removedId << "\n";
            pageIdList.pop_back();
            idPageMapping.erase(removedId);
                }
            pageIdList.push_front(id);
            idPageMapping[id] = {physicalMemPage,pageIdList.begin()};
            
        }
        //check meanwhile if some other thread already looked it up
    

        return physicalMemPage;
    }
};

int main() {
    LogicalMemory logicalMem(3); // Cache size of 3 pages

    std::vector<std::thread> threads;
    std::vector<PageId> pageIds = {"A", "B", "C", "A", "A", "A", "A", "A"};

    for (const auto& id : pageIds) {
        threads.emplace_back([&logicalMem, id]() {
            Page page = logicalMem.getPage(id);
            std::cout << "Thread fetched page: " << page.id << ", Data: " << page.data << "\n";
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    return 0;
}
//RUN: g++ -std=c++17 LogicalMemory.cpp -o LogicalMemory -lpthread