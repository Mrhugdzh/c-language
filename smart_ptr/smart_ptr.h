//
// Created by TR on 2023/2/8.
//

#ifndef C___SMART_PTR_H
#define C___SMART_PTR_H

#include<functional>

template<typename T>
class smart_ptr{
public:
    smart_ptr(); //无参构造函数
    explicit smart_ptr(T*); //有参构造函数
    smart_ptr(const smart_ptr&); // 拷贝构造函数
    smart_ptr(T*, std::function<void(T*)>); // 自定义删除器
    smart_ptr& operator=(const smart_ptr&); // 重构赋值操作符
    T& operator*() const;
    T* operator->() const;

    ~smart_ptr(); // 析构函数
    explicit operator bool() const; // 向bool类型转换

    bool unique();
    void reset();
    void reset(T*);
    void reset(T *, std::function<void(T *)>);
    T* release();

    T* get() const;

private:
    static std::function<void(T*)> default_del; // 默认的删除器

private:
    unsigned* m_p_use_count= nullptr; // 计数器指针
    T* m_p_object= nullptr; // 智能指针指向的对象
    std::function<void(T*)> m_del=default_del; // 智能指针的删除器
};

template<typename T>
std::function<void(T*)> smart_ptr<T>::default_del = [](T* p){
    delete p;
    p= nullptr;
};

//原始指针转换为智能指针
template<typename T, typename ...Args>
smart_ptr<T> make_smart(Args ...args){
    smart_ptr<T> sp(new T(std::forward<Args>(args)...));
    return sp;
}

template<typename T>
smart_ptr<T>::smart_ptr():m_p_use_count(new unsigned(1)), m_p_object(nullptr) {

}

template<typename T>
smart_ptr<T>::smart_ptr(T* p):m_p_object(p), m_p_use_count(new unsigned (1)) {

}

template<typename T>
smart_ptr<T>::smart_ptr(const smart_ptr<T> &p):m_p_object(p.m_p_object), m_p_use_count(p.m_p_use_count), m_del(p.m_del) {
    (*m_p_use_count)++;
}

template<typename T>
smart_ptr<T>::smart_ptr(T *p, std::function<void(T *)> del):m_p_object(p), m_p_use_count(new unsigned (1)), m_del(del) {

}

template<typename T>
smart_ptr<T>& smart_ptr<T>::operator=(const smart_ptr<T> &rhs) {
    // 原始指针递减，参数指针递增
    this->m_del=rhs.m_del;
    (*rhs.m_p_use_count)++;
    (*this->m_p_use_count)--;
    if(*this->m_p_use_count==0){
        m_del(m_p_object);
        delete m_p_use_count;
    }
    this->m_p_object=rhs.m_p_object;
    this->m_p_object=rhs.m_p_use_count;
    return *this;
}

template<typename T>
T& smart_ptr<T>::operator*() const {
    return *m_p_object;
}

template<typename T>
T* smart_ptr<T>::operator->() const {
    return &this->operator*();
}

template<typename T>
smart_ptr<T>::~smart_ptr() {
    if ((--m_p_use_count) == 0) {
        m_del(m_p_object);
        m_p_object= nullptr;
        delete m_p_use_count;
        m_p_use_count = nullptr;
    }
}

template<typename T>
smart_ptr<T>::operator bool() const {
    return this->m_p_object != nullptr;
}

template<typename T>
bool smart_ptr<T>::unique() {
    return *this->m_p_use_count == 1;
}

template<typename T>
void smart_ptr<T>::reset() {
    (*this->m_p_use_count)--;
    if(*this->m_p_use_count==0){
        this->m_del(this->m_p_object);
    }
    this->m_p_object= nullptr;
    this->m_p_use_count = 1;
    this->m_del = default_del;
}

template<typename T>
void smart_ptr<T>::reset(T *p) {
    (*this->m_p_use_count)--;
    if(*(this->m_p_use_count)==0){
        this->m_del(this->m_p_object);
    }
    this->m_p_object=p;
    this->m_p_use_count=1;
    this->m_del = default_del;
}

template<typename T>
void smart_ptr<T>::reset(T *p, std::function<void(T *)> del) {
    reset(p);
    this->m_del = del;
}

template<typename T>
T* smart_ptr<T>::release() {
    (*this->m_p_use_count)--;
    if (*this->m_p_use_count == 0) {
        *this->m_p_use_count = 1;
    }
    auto p = this->m_p_object;
    this->m_p_object = nullptr;
    return p;
}

template<typename T>
T* smart_ptr<T>::get() const {
    return this->m_p_object;
}

#endif //C___SMART_PTR_H
