//
// Created by TR on 2023/2/13.
//

#ifndef THREADPOOL_H
#define THREADPOOL_H

#include<iostream>
#include<mutex>
#include<condition_variable>
#include<queue>
#include<unordered_map>
#include<functional>
#include<thread>
#include<cassert>
#include<future>

namespace dpool {
    class ThreadPool { // 线程池
    public:
        using Task = std::function<void()>;
        using ThreadId = std::thread::id;
        using Thread = std::thread;
        using MutexGuard = std::lock_guard<std::mutex>;
        using UniqueGuard = std::unique_lock<std::mutex>;

        ThreadPool() : ThreadPool(Thread::hardware_concurrency()) {

        }

        explicit ThreadPool(size_t maxThreads) : quit_(false), currentThreads_(0), idleThreads_(0),
                                                 maxThreads_(maxThreads) {

        }

        // 弃用拷贝构造函数和赋值函数
        ThreadPool(const ThreadPool &) = delete;

        Thread &operator=(const ThreadPool &) = delete;

        ~ThreadPool() {
            {
                MutexGuard guard(mutex_);
                quit_ = true;
            }
            cv_.notify_all(); // 唤醒所有线程
            for (auto &elem: threads_) {
                assert(elem.second.joinable());
                elem.second.join();
            }
        }

        template<typename Func, typename ...Ts>
        auto submit(Func &&func, Ts &&... params)
        -> std::future<typename std::result_of<Func(Ts...)>::type> {
            auto execute = std::bind(std::forward<Func>(func), std::forward<Ts>(params)...);

            using ReturnType = typename std::result_of<Func(Ts...)>::type;
            using PackagedTask = std::packaged_task<ReturnType()>;

            auto task = std::make_shared<PackagedTask>(std::move(execute));  // 建立异步获取结果
            auto result = task->get_future(); // 获取任务的结果

            MutexGuard guard(mutex_); // 对资源上锁
            assert(!quit_);

            task_.template emplace([task]() {
                (*task)();
            }); // 加入任务

            if (idleThreads_ > 0) {
                cv_.notify_one(); // 唤醒一个
            } else if (currentThreads_ < maxThreads_) {  //还没有完成所有线程
                Thread t(&ThreadPool::work, this);
                assert(threads_.find(t.get_id()) == threads_.end());
                threads_[t.get_id()] = std::move(t);
                ++currentThreads_;
            }
            return result;
        }

        size_t threadNums() const{
            MutexGuard guard(mutex_);
            return currentThreads_;
        }

    private:
        void work() { // 工作任务
            while(true){
                Task task;
                {
                    UniqueGuard uniqueLock(mutex_); // 对资源上锁
                    ++idleThreads_;
                    auto hasTimeOut = !cv_.wait_for(uniqueLock,
                                                    std::chrono::seconds(WAIT_SECOND),
                                                    [this]() {
                                                        return quit_ || task_.empty();
                                                    });

                    --idleThreads_;
                    if(task_.empty()){ // 如果此时没任务了
                        if(quit_){
                            --currentThreads_;
                            return ;
                        }
                        if(hasTimeOut){ // 如果超时了
                            --currentThreads_;
                            joinFinishedThreads(); // 复用线程
                            finishedThreadIds_.emplace(std::this_thread::get_id());
                            return;
                        }
                    }
                    task = std::move(task_.front());
                    task_.pop();
                }
                task();
            }
        }

        /**
         * 线程复用
         */
        void joinFinishedThreads(){
            while(!finishedThreadIds_.empty()){
                auto id = finishedThreadIds_.front();
                finigitshedThreadIds_.pop();
                auto iter = threads_.find(id); // 找到这个线程

                assert(iter!= threads_.end());
                assert(iter->second.joinable());

                iter->second.join();
                threads_.erase(iter);
            }
        }

        static constexpr size_t WAIT_SECOND = 2;
        bool quit_;
        size_t currentThreads_;
        size_t idleThreads_;
        size_t maxThreads_;
        mutable std::mutex mutex_; // 互斥变量
        std::condition_variable cv_; // 条件变量
        std::queue<Task> task_;
        std::queue<ThreadId> finishedThreadIds_;
        std::unordered_map<ThreadId, Thread> threads_;
    };
    constexpr size_t ThreadPool::WAIT_SECOND;
}


#endif /* THREADPOOL_H */