//
// Created by Shawn Zhao on 2023/7/18.
//

#ifndef BLOCKING_CONCURRENT_QUEUE_NON_BLOCK_H
#define BLOCKING_CONCURRENT_QUEUE_NON_BLOCK_H

#include <atomic>

// Structure for node in the queue
// structure node_t {value: data type, next: pointer_t}
template<typename T>
struct Node {
    T value;
    std::atomic<Node<T>*> next;
};

//s tructure queue_t {Head: pointer_t, Tail: pointer_t}
template<typename T>
struct Queue {
    std::atomic<Node<T>*> head;
    std::atomic<Node<T>*> tail;
};

/*
initialize(Q: pointer to queue_t)
node = new_node()		// Allocate a free node
node->next.ptr = NULL	// Make it the only node in the linked list
Q->Head.ptr = Q->Tail.ptr = node	// Both Head and Tail point to it
*/
template<typename T>
void initialize(Queue<T> &Q) {
    Node<T>* node = new Node<T>();
    node -> next.store(nullptr); // store存储
    Q.haed.store(node);
    Q.tail.store(node);
}

// Enqueue an item into the queue
template<typename T>
void enqueue(Queue<T> &Q, const T &value) {
    Node<T>* node = new Node<T>();
    node -> value = value;
    node -> next.store(nullptr);
    while (true) {
        Node<T>* tail = Q.tail.load();
        Node<T>* next = tail->next.load();

        if (tail == Q.tail.load()) {
            if (next == nullptr) {
                if (std::atomic_compare_exchange_weak(&tail->next, &next, node)) {
                    // 当前值 期望值 新值
                    // 比较当前值与期望值，如果相等，
                    // 则将新值存储当前对象中，并返回 true；否则，不进行任何操作，并返回 false。
                    // weak 表示在失败时不进行重试，直接返回
                    break;
                }
            } else {
                // 更新，将tail指针指向下一个地方
                std::atomic_compare_exchange_weak(&Q.tail, &tail, next);
            }
        }
    }
}

// 在这里，head会永远会记住前一个数字，head就是一个指针，永远不会指向真的地方，head其实永远是head->next
template<typename T>
bool dequeue(Queue<T> &Q, T* pvalue) {
    while (true) {
        Node<T>* freeptr;
        Node<T>* head = Q.head.load();
        Node<T>* tail = Q.tail.load();
        Node<T>* next = head->next.load();
        if (head == Q.head.load()) {
            if (head == tail) { // empty or Tail falling behind尾巴落后
                if (next == nullptr) {
                    return false; // empty
                }
                // 将tail指向自己应该指向的地方
                std::atomic_compare_exchange_weak(&Q.tail, &tail, next);
            }
            else {
                *pvalue = next -> value;
                freeptr = head;
                if (std::atomic_compare_exchange_weak(&Q.head, &head, next)) {
                    delete freeptr;
                    break; // Dequeue done end loop
                }
            }
        }
    }
    return true;
}


#endif //BLOCKING_CONCURRENT_QUEUE_NON_BLOCK_H
