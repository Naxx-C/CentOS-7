/*
 * @Author       : mark
 * @Date         : 2020-06-17
 * @copyleft Apache 2.0
 */ 
#ifndef HEAP_TIMER_H
#define HEAP_TIMER_H

#include <queue>
#include <unordered_map>
#include <time.h>
#include <algorithm>
#include <arpa/inet.h> 
#include <functional> 
#include <assert.h> 
#include <chrono>
#include "../log/log.h"

typedef std::function<void()> TimeoutCallBack;
typedef std::chrono::high_resolution_clock Clock;   // 最高精度的时钟
typedef std::chrono::milliseconds MS;
typedef Clock::time_point TimeStamp;

struct TimerNode {
    int id;             // 客户端的文件描述符
    TimeStamp expires;  // 客户端的失效时间（绝对时间）
    TimeoutCallBack cb; // 回调函数，在不同的情况下使用不同的匿名函数
    bool operator<(const TimerNode& t) {
        return expires < t.expires;
    }
};
class HeapTimer {
public:
    HeapTimer() { heap_.reserve(64); }

    ~HeapTimer() { clear(); }
    
    void adjust(int id, int newExpires);

    void add(int id, int timeOut, const TimeoutCallBack& cb);

    void doWork(int id);

    void clear();

    void tick();

    void pop();

    int GetNextTick();

private:
    void del_(size_t i);    // 删除定时器
    
    void siftup_(size_t i); // 向上调整定时器

    bool siftdown_(size_t index, size_t n); // 向下调整定时器

    void SwapNode_(size_t i, size_t j); // 交换节点值

    std::vector<TimerNode> heap_;   // 时间堆

    std::unordered_map<int, size_t> ref_;   // 记录客户端的失效时间（只记录在时间堆中的位置，具体时间为TimerNode的expires属性）
};

#endif //HEAP_TIMER_H