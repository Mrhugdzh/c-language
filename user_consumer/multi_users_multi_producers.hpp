//
// Created by TR on 2023/2/14.
//

#ifndef USER_CONSUMER_MULTI_USERS_MULTI_PRODUCERS_HPP
#define USER_CONSUMER_MULTI_USERS_MULTI_PRODUCERS_HPP

#include<iostream>
#include<mutex>
#include<condition_variable>
#include<unistd.h>
#include <thread>

static const int KItemsRepositorySize=10;
static const int KItemsToProduce=100;

struct ItemsRepository{
    int itemBuffer[KItemsRepositorySize];
    size_t readPosition;
    size_t writePosition;
    size_t readCounter;
    size_t writeCounter;

    std::mutex mtx; // 读写互斥
    std::mutex read_mtx; // 读互斥
    std::mutex write_mtx; // 写互斥

    std::condition_variable repoNotNull;
    std::condition_variable repoNotEmpty;
}gItemsRepository;

typedef struct ItemsRepository ItemRepositort;

void initRepository(ItemsRepository *ir){
    ir->readPosition=0;
    ir->writePosition=0;
    ir->readCounter=0;
    ir->writeCounter=0;
}

void producerItem(ItemsRepository *ir, int item){
    std::unique_lock<std::mutex> lock(ir->mtx);  // 写上锁
    while((ir->writePosition+1) % KItemsRepositorySize == ir->readPosition){
       // 写空间满
       std::cout<<"Repo is full, please wait"<<std::endl;
        (ir->repoNotNull).wait(lock);
    }

    ir->itemBuffer[ir->writePosition]=item;
    (ir->writePosition)++;

    if(ir->writePosition == KItemsRepositorySize){
        ir->writePosition=0;
    }

    (ir->repoNotEmpty).notify_all();
    lock.unlock();
}

int consumeItem(ItemsRepository *ir){
    int data;
    std::unique_lock<std::mutex> lock(ir->mtx);
    while(ir->readPosition==ir->writePosition){
        // buffer是空的
        std::cout<<"Repo is empty, please wait"<<std::endl;
        (ir->repoNotEmpty).wait(lock);
    }

    data=ir->itemBuffer[ir->readPosition];
    (ir->readPosition)++;

    if(ir->readPosition==KItemsRepositorySize){
        ir->readPosition=0;
    }

    (ir->repoNotNull).notify_all();
    lock.unlock();
    return data;
}

void produceTask(){
    bool flag;
    while(true){
        sleep(1);
        std::unique_lock<std::mutex> lock(gItemsRepository.write_mtx);
        if(gItemsRepository.writeCounter>KItemsToProduce){
            flag= true;
        }else{
            producerItem(&gItemsRepository, gItemsRepository.writeCounter);
            std::cout<<"Producer thread "<<std::this_thread::get_id()<<" produce the "<<gItemsRepository.writeCounter<<" ^th item" <<std::endl;
        }
        ++(gItemsRepository.writeCounter);
        lock.unlock();
        if(flag) break;
    }
    std::cout<<"Producer thread "<<std::this_thread::get_id()<<" is exiting"<<std::endl;
}

void consumeTask(){
    bool flag= false;
    while(true){
        sleep(1);
        std::unique_lock<std::mutex> lock(gItemsRepository.read_mtx);
        if(gItemsRepository.readCounter>KItemsToProduce){
            flag= true;
        }else{
            int item= consumeItem(&gItemsRepository);
            std::cout<<"Consumer thread "<<std::this_thread::get_id()<<" consum the "<<item<< " ^th item"<<std::endl;
            (gItemsRepository.readCounter)++;
        }
        lock.unlock();
        if(flag) break;
    }
    std::cout<<"Consumer thread "<<std::this_thread::get_id()<<" is exiting"<<std::endl;
}


#endif //USER_CONSUMER_MULTI_USERS_MULTI_PRODUCERS_HPP
