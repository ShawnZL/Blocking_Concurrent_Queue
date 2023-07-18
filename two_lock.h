//
// Created by Shawn Zhao on 2023/7/18.
//

#ifndef BLOCKING_CONCURRENT_QUEUE_TWO_LOCK_H
#define BLOCKING_CONCURRENT_QUEUE_TWO_LOCK_H
#include <mutex>
template <typename T>
struct Node {
    T value;
    Node<T>* next;
};

template <typename T>
struct Queue {
    Node<T>* Head;
    Node<T>* Tail;
    std::mutex H_lock;
    std::mutex T_lock;
};

template <typename T>
void initialize(Queue<T>& Q) {
    Node<T>* node = new Node<T>();
    node->next = nullptr;
    Q.Head = Q.Tail = node;
}

template <typename T>
void enqueue(Queue<T>& Q, const T &value) {
    Node<T>* node = new Node<T>();
    node->value = value;
    node->next = nullptr;

    std::lock_guard<std::mutex> lock(Q.T_lock);
    Q.Tail -> next = node;
    Q.Tail = Q.Tail -> next;
}

template <typename T>
bool dequeue(Queue<T>& Q, T& pvalue) {
    std::lock_guard<std::mutex> lock(Q.H_lock);
    Node<T>* node = Q.Head;
    Node<T>* new_head = node -> next;
    if (new_head == nullptr) {
        return false;
    }
    pvalue = node->value;
    Q.Head = new_head;
    delete node;
    return true;
}

#endif //BLOCKING_CONCURRENT_QUEUE_TWO_LOCK_H
