#include <iostream>
#include "multi_users_multi_producers.hpp"
//#include "single_user_single_consumer.hpp"
//#include "multi_user_single_producer.hpp"
//#include "single_user_multi_producers.hpp"
int main() {
    //// 单生产者——单消费者
    ////1. 初始化仓库
    //InitItemRepository(&gItemRepository);
    ////2. 创建生产者线程
    //std::thread producer(ProduceTask);
    ////3. 创建消费者线程
    //std::thread consumer(ConsumerTask);
    ////4. 启动线程
    //producer.join();
    //consumer.join();

    // 单生产者——多消费者  多人读，要保证读的顺序，存储一个计数器，读到了哪里
    //InitRepository(&gItemRepository);
    //std::thread produce(ProdcerTask);
    //std::thread consume1(ConsumerTask);
    //std::thread consume2(ConsumerTask);
    //std::thread consume3(ConsumerTask);
    //std::thread consume4(ConsumerTask);
    //produce.join();
    //consume1.join();
    //consume2.join();
    //consume3.join();
    //consume4.join();

    // 多生产者——单消费者  多人写，要保证写不重复，需要一个记录写的计数器，存储写到了哪里
    //initRepository(&gItemRepository);
    //std::thread producer1(ProduceTask);
    //std::thread producer2(ProduceTask);
    //std::thread producer3(ProduceTask);
    //std::thread producer4(ProduceTask);
    //std::thread consumer(ConsumeTask);
    //
    //producer1.join();
    //producer2.join();
    //producer3.join();
    //producer4.join();
    //consumer.join();

    // 多生产者——多消费者
    initRepository(&gItemsRepository);
    std::thread producer1(produceTask);
    std::thread producer2(produceTask);
    std::thread producer3(produceTask);
    std::thread producer4(produceTask);
    std::thread consume1(consumeTask);
    std::thread consume2(consumeTask);
    std::thread consume3(consumeTask);
    std::thread consume4(consumeTask);
    producer1.join();
    producer2.join();
    producer3.join();
    producer4.join();
    consume1.join();
    consume2.join();
    consume3.join();
    consume4.join();


    return 0;
}
