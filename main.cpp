#include <iostream>
#include "skiplist.h"

using namespace std;

int main() {
    SkipList<int, string> sp(10);
    sp.insert_value(0, "I");
    sp.insert_value(1, "am");
    sp.insert_value(2, "hugang");
    sp.insert_value(3, "now");
    sp.insert_value(4, "learning");
    sp.insert_value(5, "skipList");

    sp.display();

    

    return 1;
}