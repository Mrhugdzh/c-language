#include <iostream>
#include "skiplist.h"

using namespace std;

int main() {
    SkipList<string, string> sp(10);
    // sp.insert_value(0, "I");
    // sp.insert_value(1, "am");
    // sp.insert_value(2, "hugang");
    // sp.insert_value(3, "now");
    // sp.insert_value(4, "learning");
    // sp.insert_value(5, "skipList");

    // sp.display();

    // sp.delete_value(0);

    // sp.display();

    // sp.delete_value(3);

    // sp.display();

    // cout<<sp.search(5)<<endl;
    // sp.search(0);

    // sp.set_value(2, "Liming");
    // sp.display();

    // sp.dump_file();

    sp.load_file();
    sp.display();


    

    

    return 1;
}