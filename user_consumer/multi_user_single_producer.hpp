//
// Created by TR on 2023/2/14.
//

#ifndef USER_CONSUMER_MULTI_USER_SINGLE_PRODUCER_HPP
#define USER_CONSUMER_MULTI_USER_SINGLE_PRODUCER_HPP

#include<thread>
#include<mutex>
#include<condition_variable>
#include<unistd.h>

static const int KItemsRepositorySize=4; // 仓库数量
static const int KItemsToProduce=100; // 总共生产的数量

struct ItemRepository{
    int item_buffer[KItemsRepositorySize]; // 仓库大小
    size_t read_position;
    size_t write_position;
    size_t item_counter;

    std::mutex mtx;  // 读写操作的互斥变量
    std::mutex item_counter_mtx; // 读操作的互斥变量

    std::condition_variable repo_not_full;
    std::condition_variable repo_not_empty;
}gItemRepository;

typedef struct ItemRepository ItemRepository;

/**
 * 生产
 * @param ir
 * @param item
 */
void ProducerItem(ItemRepository* ir, int item){
    std::unique_lock<std::mutex> lock(ir->mtx); // 先对读写上锁
    // 仓库满了
    //std::cout << ir->write_position << std::endl;
    while((ir->write_position+1) % KItemsRepositorySize == ir->read_position){
        std::cout<<"Repo is full, please wait"<<std::endl;
        (ir->repo_not_full).wait(lock);
    }

    ir->item_buffer[ir->write_position]=item;
    (ir->write_position)++;

    if(ir->write_position == KItemsRepositorySize){
        ir->write_position=0;
    }

    (ir->repo_not_empty).notify_all();
    lock.unlock();
}

/**
 * 消费
 * @param ir
 * @return
 */
int ConsumerItem(ItemRepository* ir){
    std::unique_lock<std::mutex> lock(ir->mtx);
    while(ir->read_position == ir->write_position){
        std::cout<<"Repo is empty,please wait"<<std::endl;
        (ir->repo_not_empty).wait(lock);
    }

    int data = ir->item_buffer[ir->read_position];
    (ir->read_position)++;

    if(ir->read_position==KItemsRepositorySize){
        ir->read_position=0;
    }

    ir->repo_not_full.notify_all();
    lock.unlock();

    return data;
}

/**
 * 生产者任务
 */
void ProdcerTask(){
    for(int i=0;i<KItemsToProduce;i++){
        std::cout<<"Produce thread "<<std::this_thread::get_id()<<" produce the "<<i<<" ^th item"<<std::endl;
        ProducerItem(&gItemRepository, i);
    }
    std::cout<<"Producer thread"<<std::this_thread::get_id()<<" is exiting"<<std::endl;
}

/**
 * 消费者任务
 */
void ConsumerTask(){
    int data;
    bool flag= false;
    while(1){
        sleep(1);
        std::unique_lock<std::mutex> lock(gItemRepository.item_counter_mtx);
        if(gItemRepository.item_counter<KItemsToProduce){
            // 能够开始读
            int item = ConsumerItem(&gItemRepository);
            (gItemRepository.item_counter)++;
            std::cout<<"Consumer thread "<<std::this_thread::get_id()<<" consume the"<<item<<"^th item"<<std::endl;
        }else{
            flag= true;
        }
        lock.unlock();
        if(flag) break;
    }
    std::cout<<"Consumer thread"<<std::this_thread::get_id()<<" is exiting……"<<std::endl;
}

/**
 * 初始化仓库
 * @param ir
 */
void InitRepository(ItemRepository* ir){
    ir->read_position=0;
    ir->write_position=0;
    ir->item_counter=0;
}

#endif //USER_CONSUMER_MULTI_USER_SINGLE_PRODUCER_HPP
