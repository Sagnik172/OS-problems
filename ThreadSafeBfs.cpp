#include <vector>
#include <queue>
#include <thread>
#include <unordered_set>
#include <iostream>
#include <mutex>
using namespace std;
class Graph{
    private:
    vector<vector<int>> edges;

    public:
    Graph(int n){
        edges.assign(n,{});
    }

    void addEdge(int a,int b){
        edges[a].push_back(b);
        edges[b].push_back(a);
    }
/*

bfs 3->{4,5,6,7,8,9}
process(4)
process(5)
pr
*/
    void runBfs(int startNode){
        queue<int> q;
        q.push(startNode);
        unordered_set<int> visited;
        vector<int> currentFrontier;
        mutex mtx;
        visited.insert(startNode);
        currentFrontier.push_back(startNode);


        unsigned int numThreads = thread::hardware_concurrency();
        if(numThreads==0){
            numThreads = 4; // Default to 4 threads if hardware_concurrency cannot determine the number of threads
        }
        int level = 0;
        while(!currentFrontier.empty()){
            cout<<"Processing level "<<level<<" with current frontier size : "<<currentFrontier.size()<<"\n";
            int currentChunkSize = (static_cast<int>(currentFrontier.size())+numThreads-1)/numThreads;
            vector<thread> threads;
            vector<int> nextFrontier;
            for(int i=0;i<numThreads;i++){
                int startIdx = i * currentChunkSize;
                int endIdx = min(static_cast<int>(currentFrontier.size()),startIdx+currentChunkSize);
                if(startIdx>=endIdx){
                    break; // No more nodes to process for this thread
                }
                threads.emplace_back([&,startIdx,endIdx]{
                    vector<int> localFrontier;
                    for(int j = startIdx;j<endIdx;j++){
                        int currNode = currentFrontier[j];
                        for(int child:edges[currNode]){
                            {
                                lock_guard<std::mutex> lg(mtx);
                                if(visited.find(child) == visited.end()){
                                    localFrontier.push_back(child);
                                    visited.insert(child);
                                }

                            }
                        }
                    }

                    {
                        lock_guard<mutex> lg(mtx);
                        nextFrontier.insert(nextFrontier.end(),localFrontier.begin(),localFrontier.end());
                    }
                });
            }
            for(auto &th:threads){
                th.join();
            }
            level++;
            currentFrontier = move(nextFrontier);
        }
    }
};

int main(){

    Graph pg(10);
    
    // Creating a simple tree-like graph
    pg.addEdge(1, 2);
    pg.addEdge(1, 3);
    pg.addEdge(2, 4);
    pg.addEdge(2, 5);
    pg.addEdge(3, 6);
    pg.addEdge(3, 7);
    pg.addEdge(4, 8);
    pg.addEdge(7, 9);

    std::cout << "Starting Parallel BFS...\n";
    pg.runBfs(1);
    std::cout << "BFS Complete.\n";

    return 0;

}
//RUN: g++ -std=c++17 -pthread ThreadSafeBfs.cpp -o ThreadSafeBfs