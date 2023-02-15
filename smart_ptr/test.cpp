//
// Created by TR on 2023/2/8.
//

#include "smart_ptr.h"
#include<iostream>

struct Mystruct{
    Mystruct()=default;
    Mystruct(int a, int b){
        this->a = a;
        this->b = b;
    }
    int a;
    int b;
};

int main(){
    auto mystruct = new Mystruct();
    mystruct->a=10;
    mystruct->b=10;

    smart_ptr<Mystruct> sp(mystruct);
    std::cout<<sp->a<<std::endl;
    std::cout << sp->b << std::endl;

    std::cout<<(*sp).a<<std::endl;

    auto sp2 = make_smart<Mystruct>(100, 200);
    std::cout<<sp2->a<<std::endl;
    std::cout<<sp2->b<<std::endl;

    auto p = sp2.release();
    std::cout<<p->a<<std::endl;
    std::cout << p->b << std::endl;

    return 0;
}