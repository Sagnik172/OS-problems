#include <list>
#include <unordered_map>
#include <stdexcept>
/*
The Limitation of Standard LRU:
 The "One-Time Scan" ProblemImagine you have a standard LRU cache holding your most frequently accessed, highly valuable database records. Your application is running smoothly with a 95% cache hit rate.Then, a background task triggers a full database backup, or an analyst runs a massive SELECT * analytics query.In a standard LRU:Millions of records are read from the disk.Every single record is pushed to the Most Recently Used (MRU) end of the cache.Because the cache has a fixed size, this massive influx of new data forces all of your genuinely "hot" (frequently accessed) data out the LRU end.By the time the backup finishes, your cache is completely filled with data that will probably never be read again. Your cache hit rate drops to 0%, your database gets hammered by normal user traffic, and your application performance tanks until the cache slowly "warms up" again.Standard LRU cannot distinguish between data that is truly popular and data that just happened to be read a millisecond ago.How Your Segmented LRU Resolved ThisSegmented LRU solves this by demanding proof of popularity. It treats the cache like a nightclub with a VIP section.The Probationary Segment (Untouched - $n/3$):When new data is read, it is placed here. If a massive database scan happens, it will churn through this segment. The data enters, moves to the LRU end, and gets evicted quickly. It acts as a sacrificial buffer.The Protected Segment (Touched - $2n/3$):To get into this larger segment, a piece of data must be requested a second time while it is still in the probationary segment. This second request is the "proof" that the data is actually useful.The ResultIf an analyst runs a massive scan, that data only ever touches the $n/3$ segment and is quickly discarded. Your VIP data sitting in the $2n/3$ segment is completely shielded from the scan. The cache pollution is contained, and your application's performance remains stable.This architecture is so effective that variants of it are used in the Linux Kernel page cache and modern database engines like PostgreSQL.
*/

struct LRUNode{
    int key;
    int value;
    bool is_touched;
    LRUNode(int k,int v,bool _touched):key(k),value(v),is_touched(_touched){}
};

class LRU{

    public:
    LRU(int cap1,int cap2):untouched_list_cap(cap1),touched_list_cap(cap2){}

    void insert(int key,int val){
        auto it = cache.find(key);
        if(it!=cache.end()){
           //in some list
           LRUNode& node = *(it->second);
           bool is_touched = node.is_touched;
           node.value = val;
           if(!is_touched){
            //present in first list,move to mru end of untouched list
   
            untouched_list.splice(untouched_list.begin(),untouched_list,it->second);
            cache[key] = untouched_list.begin();

           }
           else{
            //already in touched
                touched_list.splice(touched_list.begin(),touched_list,it->second);
                cache[key] = touched_list.begin();

           }
           check_and_evict();
            return;
        }
        //not there , put in mru side of untouched list
        LRUNode addNode = LRUNode(key,val,false);
        untouched_list.emplace_front(addNode);
        cache[key] = untouched_list.begin();
        check_and_evict();


    }

    int get(int key){
        auto it = cache.find(key);
        if(it == cache.end()){
            throw std::runtime_error("Element not found in cache");
        }
        auto list_iter = it->second;
        LRUNode& node = *list_iter;
        bool is_touched = node.is_touched;
        if(!is_touched){
            node.is_touched = true;
            touched_list.splice(touched_list.begin(),untouched_list,list_iter);
        }
        else{
            touched_list.splice(touched_list.begin(),touched_list,list_iter);
        }
        cache[key] = touched_list.begin();
        check_and_evict();
        int res = cache[key]->value;
        return res;
    }


    private:
    void check_and_evict(){
        if(untouched_list.size()>untouched_list_cap){
            evict(untouched_list);
        }
        if(touched_list.size()>touched_list_cap){
            evict(touched_list);
        }
    }
    void evict(std::list<LRUNode> &list){
        auto it = list.back();
        int key = it.key;
        cache.erase(key);
        list.pop_back();
    }

    std::list<LRUNode> touched_list,untouched_list;
    std::unordered_map<int,std::list<LRUNode>::iterator> cache;
    int untouched_list_cap,touched_list_cap;

};