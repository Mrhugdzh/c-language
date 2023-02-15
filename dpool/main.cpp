#include "ThreadPool.hpp"

#include <iostream>

using namespace dpool;

std::mutex countMtx;

void task(int taskId)
{
    {
        std::lock_guard<std::mutex> guard(countMtx);
        std::cout << "task-" << taskId << " begin!" << std::endl;
    }

    // executing task for 2 second
    std::this_thread::sleep_for(std::chrono::seconds(2));

    {
        std::lock_guard<std::mutex> guard(countMtx);
        std::cout << "task-" << taskId << " end!" << std::endl;
    }
}

void mointor(const ThreadPool &pool, int seconds){
    for(int i=1;i<seconds*10;i++){
        {
            std::lock_guard<std::mutex> guard(countMtx);
            std::cout << "thread num: " << pool.threadNums() << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

int compute(int a, int b){
    return a + b;
}

int main(int argc, char *argv[])
{
    //ThreadPool pool(100);
    //
    //pool.submit(mointor, std::ref(pool), 1);
    //
    //for(int i=0; i<100;i++){
    //    std::this_thread::sleep_for(std::chrono::microseconds(100));
    //    pool.submit(task, i);
    //}

    ThreadPool pool(10);
    auto ful = pool.submit(compute, 100, 100);
    std::cout<<ful.get()<<std::endl;

    return 0;
}