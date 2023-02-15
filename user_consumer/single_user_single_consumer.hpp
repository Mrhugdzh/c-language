//
// Created by TR on 2023/2/14.
//

#ifndef USER_CONSUMER_SINGLE_USER_SINGLE_CONSUMER_HPP
#define USER_CONSUMER_SINGLE_USER_SINGLE_CONSUMER_HPP

#include<mutex>
#include<condition_variable>
#include<cstddef>
#include<unistd.h>
#include<thread>
#include <iostream>

static const int KItemsRepositorySize = 10; // 仓库最大数量
static const int KItemsToProduce = 1000; // 预计生产的数量

// 仓库结构体
struct ItemRepository{
    int item_buffer[KItemsToProduce]; // 产品缓冲区
    size_t read_position;
    size_t write_position;

    std::mutex mtx; // 互斥量，保护产品缓冲区

    std::condition_variable repo_not_full; //条件变量1：缓冲区不为满
    std::condition_variable repo_not_empty; // 条件变量2：缓冲区不为空
}gItemRepository; // 产品库全局变量，生产者和消费者使用

typedef struct ItemRepository ItemRepository;

/**
 * 向仓库中进行生产
 * @param ir
 * @param item
 */
void ProduceItem(ItemRepository *ir, int item){
    std::unique_lock<std::mutex> lock(ir->mtx);// 对仓库上锁
    while((ir->write_position +1) % KItemsRepositorySize == ir->read_position){
        // 此时buffer是满的，等待
        std::cout<<"Repository is full, please wait"<<std::endl;
        (ir->repo_not_full).wait(lock);
    }

    // buffer 不满
    (ir->item_buffer)[ir->write_position] = item; // 存入商品
    (ir->write_position)++;

    if(ir->write_position == KItemsRepositorySize){
        ir->write_position = 0;
    }

    // 通知消费者，仓库不为空
    (ir->repo_not_empty).notify_all();
    lock.unlock();
}

/**
 * 从仓库中取出商品
 * @param ir
 * @return
 */
int ConsumItem(ItemRepository *ir){
    std::unique_lock<std::mutex> lock(ir->mtx); // 对仓库上锁
    while(ir->read_position+1 == ir->write_position){
        // 仓库为空
        std::cout<<"Repository is empty, please wait"<<std::endl;
    }

    int data;
    data = ir->item_buffer[ir->read_position];
    (ir->read_position)++;

    if(ir->read_position==KItemsRepositorySize){
        ir->read_position = 0;
    }

    (ir->repo_not_full).notify_all(); // 通知生产者，仓库不满
    lock.unlock();

    return data;
}

// 消费者任务
void ConsumerTask(){
    static int cnt = 0;
    while(1){
        sleep(1);
        int item = ConsumItem(&gItemRepository); // 获得物品
        std::cout<<"Consume the"<<item<<"^th item"<<std::endl;
        if(++cnt == KItemsToProduce)
            break;
    }
}

// 生产者任务
void ProduceTask(){
    for(int i=0;i < KItemsToProduce;++i){
        std::cout<<"Produce the"<<i<<" ^th item..."<<std::endl;
        ProduceItem(&gItemRepository, i);
    }
}

//初始化仓库
void InitItemRepository(ItemRepository *ir){
    ir->read_position=0;
    ir->write_position = 0;
}

#endif //USER_CONSUMER_SINGLE_USER_SINGLE_CONSUMER_HPP
