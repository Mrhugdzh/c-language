#include <iostream>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <fstream>

using namespace std;

mutex mtx;
string store_file="./store/dumpFile";

template<typename K, typename V>
class Node{
    public:
        Node(){}

        Node(K k, V v, int);

        ~Node();

        K get_key() const;

        V get_value() const;

        void set_value(V v);

        // 用于存储后续节点，存储的是指向Node*的数�?
        Node<K, V> **forward;

        int node_level;

    private:
        K key;
        V value;
};

template<typename K, typename V>
Node<K, V>::Node(K k, V v, int level):key(k), value(v), node_level(level){
    /* forward是一个数组，其中每一个元素都指向的是Node* */
    this->forward = new Node<K, V>*[node_level+1];
    memset(this->forward, 0, sizeof(Node<K, V>*) * (node_level+1));
}


template<typename K,typename V>
Node<K, V>::~Node(){
    delete []forward;
}

template<typename K, typename V>
K Node<K, V>::get_key() const{
    return this->key;


}

template<typename K, typename V>
V Node<K, V>::get_value() const{
    return this->value;
}

template<typename K, typename V>
void Node<K, V>::set_value(V v) {
    this->value = v;
}


template<typename K, typename V>
class SkipList{
    public:
        SkipList(int max_level);
        ~SkipList();
        
        /*定义增删改查*/
        bool insert_value(K k, V v);
        bool delete_value(K v);
        bool set_value(K k, V v);
        void display();
        V search(K k);

        void dump_file();
        void load_file();

        int size();
        Node<K, V>* create_node(K, V, int);

    private:
        int get_random_level();
        void get_key_value_from_line(string line, string *key, string *value);
        bool is_valid(const string str);

    private:
        int _max_level; // 跳表最大的�?
        int _current_level; // 跳表当前的层�?
        Node<K, V>* _header; // 头结�?
        int _node_num; // 当前跳表的节点个�?

        std::ofstream _file_writer;
        std::ifstream _file_reader;
};

template<typename K, typename V>
Node<K, V>* SkipList<K, V>::create_node(K k, V v, int level) {
    Node<K, V>* node = new Node<K, V>(k, v, level);
    return node;
}

template<typename K, typename V>
int SkipList<K, V>::size(){
    return _node_num;
}

template<typename K, typename V>
SkipList<K, V>::SkipList(int max_level):_max_level(max_level){
    this->_current_level = 0;
    this->_node_num = 0;

    K k;
    V v;
    this->_header = new Node<K, V>(k, v, _max_level);
}

template<typename K, typename V>
SkipList<K, V>::~SkipList(){
    if(this->_file_writer.is_open()) {
        this->_file_writer.close();
    }

    if(this->_file_reader.is_open()) {
        this->_file_reader.close();
    }

    delete _header;
}

template<typename K, typename V>
int SkipList<K, V>::get_random_level(){
    int k = 1;
    while(rand() % 2 != 0) {
        k++;
    }
    if(k>=this->_max_level) {
        k = this->_max_level;
    }
    return k;
}

/**
 * 打印跳表
*/
template<typename K, typename V>
void SkipList<K, V>::display(){
    cout<<"\n**********Skip List**********\n";
    for(int i=0;i<=this->_current_level;i++){
        /*获取这一行的头部节点*/
        Node<K, V>* node = this->_header->forward[i];
        cout<<"Level "<<i<<" : ";
        while(node!=nullptr) {
            cout<<node->get_key()<<":"<<node->get_value()<<"   ";
            node = node->forward[i];
        }
        cout<<endl;
    }
}

template<typename K, typename V>
bool SkipList<K, V>::insert_value(K k, V v){
    // 首先要确保key不存�?
    mtx.lock();
    Node<K, V>* current = this->_header;

    Node<K, V>* update[this->_max_level+1];
    for(int i=this->_current_level; i>=0; i--) {
        while(current->forward[i] != nullptr && current->forward[i]->get_key() < k) {
            current = current->forward[i];
        }
        update[i]=current;
    }

    // 判断key是否存在
    current = current->forward[0];
    if(current!=nullptr &&  current->get_key() == k) {
        cout<<"key "<<k<<" is existing\n";
        mtx.unlock();
        return 0;
    }

    // key不存在
    if(current == nullptr || current->get_key()!=k){
        
        int level = this->get_random_level();
        Node<K, V>* node = this->create_node(k, v, level);
        
        if(level > this->_current_level) {
            for(int i = this->_current_level+1; i<level+1; i++) {
                update[i] = _header;
            }
            _current_level = level;
        }

        for(int i = level; i>=0; i--) {
            node->forward[i] = update[i]->forward[i];
            update[i]->forward[i] = node;
        }
 
        cout<<"insert key "<<k<<" value v "<<v<<"  successfully\n";
        ++this->_node_num;
    }
    mtx.unlock();
    return 1;
}

template<typename K, typename V>
bool SkipList<K, V>::delete_value(K k) {
    mtx.lock();
    Node<K, V>* current = this->_header;
    Node<K,V>* update[this->_max_level+1];
    memset(update, 0, sizeof(Node<K, V>*) * (this->_max_level+1));
    for(int i=this->_current_level;i>=0;i--){
        while(current->forward[i]!=nullptr && current->forward[i]->get_key()<k) {
            
            current = current->forward[i];
        }

        update[i]=current;
    }

    current=current->forward[0]; // 找到待删除的节点
    /*current==nullptr 没有这个节点*/
    /*current->get_key()!=k 没有这个节点*/
    if(current!=nullptr || current->get_key()==k) {
        //需要进行删除
        for(int i=0; i<=this->_current_level; i++) {
            if(update[i]->forward[i]!=current) { // 当前层没有，那么上面的所有层都没有
                break;
            }
            update[i]->forward[i]=current->forward[i];
        }

        while(_current_level>0 && this->_header->forward[_current_level]==0) {
            --_current_level;
        }

        --_node_num;
    }
    mtx.unlock();
    return 0;
}

template<typename K, typename V>
V SkipList<K, V>::search(K k) {
    V result;
    Node<K, V>* current = this->_header;
    for(int i=this->_current_level; i>=0; i--) {
        while(current->forward[i]!=nullptr && current->forward[i]->get_key()<k){
            current = current->forward[i];
        }
    }

    current = current->forward[0];
    if(current->get_key()==k) {
        result = current->get_value();
    } else {
        cout<<"There is no key "<<k<<endl;
    }
    return result;
}

template<typename K, typename V>
bool SkipList<K, V>::set_value(K k, V v) {
    Node<K, V>* current = this->_header;
    for(int i=this->_current_level; i>=0; i--) {
        while(current->forward[i]!=nullptr && current->forward[i]->get_key()<k){
            current = current->forward[i];
        }
    }

    current = current->forward[0];
    if(current->get_key()==k) {
        current->set_value(v);
        cout<<"set key "<<k<<" to value "<<v<<endl;
        return 1;
    } else {
        cout<<"There is no key "<<k<<endl;
    }
    return 0;
}

template<typename K, typename V>
bool SkipList<K, V>::is_valid(const string str) {
    if(str.size()==0) {
        return 0;
    }
    if(str.find(":") == string::npos) {
        return false;
    }
    return true;
}

template<typename K, typename V>
void SkipList<K, V>::get_key_value_from_line(string line, string *key, string *value) {
    if(!is_valid(line)) {
        cout<<"line is valid\n";
        return ;
    }

    *key = line.substr(0, line.find(":"));
    cout<<key<<endl;
    *value = line.substr(line.find(":"), line.size()-1);
}


template<typename K, typename V>
void SkipList<K, V>::dump_file(){
    _file_writer.open(store_file);
    Node<K, V>* current = this->_header->forward[0];

    while(current!=nullptr) {
        _file_writer<<current->get_key()<<":"<<current->get_value()<<"\n";
        current=current->forward[0];
    }
    _file_writer.flush();
    _file_writer.close();
}

template<typename K, typename V>
void SkipList<K, V>::load_file() {
    _file_reader.open(store_file);
    string line;
    string *key= new std::string();
    string *value= new std::string();
    while(getline(_file_reader, line)){
        get_key_value_from_line(line, key, value);
        if(key->empty() || value->empty()) {
            continue;
            
        }
        this->insert_value(*key, *value);
    }
    _file_reader.close();
}
