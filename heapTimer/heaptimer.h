#ifndef HEAPTIMER_H
#define HEAPTIMER_H

#include<functional>
#include<chrono>
#include<vector>
#include<assert.h>
#include<unordered_map>

typedef std::function<void()> TimeroutCallBack;
typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::milliseconds MS;
typedef Clock::time_point TimeStamp;


struct TimerNode{
    int id;
    TimeStamp expires;
    TimeroutCallBack cb;
    bool operator<(const TimerNode& t){
        return expires < t.expires;
    }
};

class HeapTimer{
public:
    HeapTimer();
    ~HeapTimer();

    void adjust(int id,int newExpires);

    void add(int id,int timeOut,const TimeroutCallBack& cb);

    bool empty(){
        if(heap_.empty()){
            return true;
        }
        return false;
    }

    void doWork(int id);

    void clear();

    void tick();

    void pop();

    int GetNextTick();

private:

    void del_(size_t i);

    void siftup_(size_t i);

    bool siftdown_(size_t index,size_t n);

    void SwapNode_(size_t i,size_t j);

    std::vector<TimerNode> heap_;

    std::unordered_map<int,size_t> ref_;

};





#endif