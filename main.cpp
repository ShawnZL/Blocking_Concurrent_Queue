#include <iostream>
//#include "Non_Block.h"
#include "two_lock.h"
int main() {
    /*
    // 创建一个队列
    Queue<int> queue;
    queue.head.store(new Node<int>());
    queue.tail.store(queue.head.load());

    // 入队操作
    for (int i = 1; i < 5; ++i) {
        enqueue(queue, i);
    }

    // 出队操作
    int value;
    while (dequeue(queue, &value)) {
        std::cout << "Dequeued value: " << value << std::endl;
    }
    */

    Queue<int> q;
    initialize(q);

    // 入队操作
    enqueue(q, 10);
    enqueue(q, 20);
    enqueue(q, 30);

    // 出队操作
    int value;
    while (dequeue(q, value)) {
        std::cout << "Dequeued value: " << value << std::endl;
    }

    return 0;
}
