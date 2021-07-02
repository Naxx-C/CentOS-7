/*
 * @Author       : mark
 * @Date         : 2020-06-15
 * @copyleft Apache 2.0
 */ 

#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <mutex>
#include <condition_variable>
#include <queue>
#include <thread>
#include <functional>
class ThreadPool {
public:
    explicit ThreadPool(size_t threadCount = 8): pool_(std::make_shared<Pool>()) {
            assert(threadCount > 0);
            // 创建threadCount个子线程
            for(size_t i = 0; i < threadCount; i++) {
                std::thread([pool = pool_] {
                    std::unique_lock<std::mutex> locker(pool->mtx);
                    while(true) {
                        // 任务队列不为空
                        if(!pool->tasks.empty()) {
                            auto task = std::move(pool->tasks.front()); // 使用移动语义拿到任务队列的第一个元素
                            pool->tasks.pop();
                            locker.unlock(); //将线程池解锁允许本线程执行任务时,其他线程拿到任务队列的元素，提高并发
                            task(); // 执行任务队列中的任务
                            locker.lock();  // 执行完任务后继续上锁，保持unique_lock的特性,继续拿下一个任务
                        } 
                        else if(pool->isClosed) break;
                        else pool->cond.wait(locker); // 等待任务到来
                    }
                }).detach();
            }
    }

    ThreadPool() = default;

    ThreadPool(ThreadPool&&) = default;
    
    ~ThreadPool() {
        if(static_cast<bool>(pool_)) {
            {
                std::lock_guard<std::mutex> locker(pool_->mtx);
                pool_->isClosed = true;
            }
            pool_->cond.notify_all();
        }
    }

    template<class F>
    void AddTask(F&& task) {
        {
            std::lock_guard<std::mutex> locker(pool_->mtx);
            pool_->tasks.emplace(std::forward<F>(task)); // std::forward()完美转发，右值一直为右值，左值一直为左值
        }
        pool_->cond.notify_one(); // 唤醒线程
    }

private:
    // 线程池 结构体
    struct Pool {
        std::mutex mtx; // 互斥锁
        std::condition_variable cond; // 条件变量
        bool isClosed;  // 是否关闭
        std::queue<std::function<void()>> tasks; // 队列，保存的是任务（函数对象）
    };
    std::shared_ptr<Pool> pool_; // 池子
};


#endif //THREADPOOL_H