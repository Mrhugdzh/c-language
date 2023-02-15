//
// Created by TR on 2023/2/14.
//

#ifndef USER_CONSUMER_SINGLE_USER_MULTI_PRODUCERS_HPP
#define USER_CONSUMER_SINGLE_USER_MULTI_PRODUCERS_HPP

#include<thread>
#include<mutex>
#include<condition_variable>
#include<iostream>
#include<unistd.h>

static const int KItemsRepositorySize=5;
static const int KItemsToProduce=100;

struct ItemReposityoy{
    int itemBuffer[KItemsRepositorySize];
    size_t readPosition;
    size_t writePosition;
    size_t itemWriteCounter;

    std::mutex mtx;
    std::mutex writeCounterMtx;

    std::condition_variable repoNotFull;
    std::condition_variable repoNotEmpty;
}gItemRepository;

void ProduceItem(ItemReposityoy *ir, int item){
    std::unique_lock<std::mutex> lock(ir->mtx); // 对写上锁
    while((ir->writePosition+1) % KItemsRepositorySize == ir->readPosition){
        // 此时已经满了
        std::cout<<"Repo is full, please wait"<<std::endl;
        (ir->repoNotFull).wait(lock);
    }

    ir->itemBuffer[ir->writePosition]=item;
    (ir->writePosition)++;

    if(ir->writePosition == KItemsRepositorySize){
        ir->writePosition=0;
    }

    (ir->repoNotEmpty).notify_all();
    lock.unlock();
}

int ConsumerItem(ItemReposityoy *ir){
    std::unique_lock<std::mutex> lock(ir->mtx);
    while(ir->readPosition == ir->writePosition){
        std::cout << "Repo is empty, please wait" << std::endl;
        (ir->repoNotEmpty).wait(lock);
    }
    int data = ir->itemBuffer[ir->readPosition];
    (ir->readPosition)++;

    if(ir->readPosition==KItemsRepositorySize){
        ir->readPosition=0;
    }

    (ir->repoNotFull).notify_all();
    lock.unlock();
    return data;
}

void ProduceTask(){
    bool flag=false;
    while(true){
        sleep(1);
        std::unique_lock<std::mutex> lock(gItemRepository.writeCounterMtx); // 给写上锁
        if(gItemRepository.itemWriteCounter<KItemsToProduce){
            // 没有写完
            // 进行写
            ProduceItem(&gItemRepository, gItemRepository.itemWriteCounter);
            std::cout<<"Producer thread "<<std::this_thread::get_id()<<" produce the"<<gItemRepository.itemWriteCounter<<" ^th item"<<std::endl;
            (gItemRepository.itemWriteCounter)++;
        }else{
            flag=true;
        }
        lock.unlock();
        if(flag) break;
    }
    std::cout << "Producer thread " << std::this_thread::get_id() << " is exiting" << std::endl;
}

void ConsumeTask(){
    static int counter=0;
    while(true){
        sleep(1);
        int item = ConsumerItem(&gItemRepository);
        if(counter>KItemsToProduce){
            break;
        }
        std::cout << "Consumer thread " << std::this_thread::get_id() << " consume the" << item << " ^th item"
                  << std::endl;
        ++counter;
    }
}

void initRepository(ItemReposityoy* ir){
    ir->readPosition=0;
    ir->writePosition=0;
    ir->itemWriteCounter=0;
}

#endif //USER_CONSUMER_SINGLE_USER_MULTI_PRODUCERS_HPP
