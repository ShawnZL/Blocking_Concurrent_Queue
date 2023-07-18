# 伪代码

[高性能多线程队列链接](https://www.cs.rochester.edu/research/synchronization/pseudocode/queues.html)

## Non-Blocking Concurrent Queue Algorithm

```
structure pointer_t {ptr: pointer to node_t, count: unsigned integer}
  structure node_t {value: data type, next: pointer_t}
  structure queue_t {Head: pointer_t, Tail: pointer_t}
  
  initialize(Q: pointer to queue_t)
     node = new_node()		// Allocate a free node
     node->next.ptr = NULL	// Make it the only node in the linked list
     Q->Head.ptr = Q->Tail.ptr = node	// Both Head and Tail point to it
  
  enqueue(Q: pointer to queue_t, value: data type)
   E1:   node = new_node()	// Allocate a new node from the free list
   E2:   node->value = value	// Copy enqueued value into node
   E3:   node->next.ptr = NULL	// Set next pointer of node to NULL
   E4:   loop			// Keep trying until Enqueue is done
   E5:      tail = Q->Tail	// Read Tail.ptr and Tail.count together
   E6:      next = tail.ptr->next	// Read next ptr and count fields together
   E7:      if tail == Q->Tail	// Are tail and next consistent?
               // Was Tail pointing to the last node?
   E8:         if next.ptr == NULL
                  // Try to link node at the end of the linked list
   E9:            if CAS(&tail.ptr->next, next, <node, next.count+1>)
  E10:               break	// Enqueue is done.  Exit loop
  E11:            endif
  E12:         else		// Tail was not pointing to the last node
                  // Try to swing Tail to the next node
  E13:            CAS(&Q->Tail, tail, <next.ptr, tail.count+1>)
  E14:         endif
  E15:      endif
  E16:   endloop
         // Enqueue is done.  Try to swing Tail to the inserted node
  E17:   CAS(&Q->Tail, tail, <node, tail.count+1>)
  
  dequeue(Q: pointer to queue_t, pvalue: pointer to data type): boolean
   D1:   loop			     // Keep trying until Dequeue is done
   D2:      head = Q->Head	     // Read Head
   D3:      tail = Q->Tail	     // Read Tail
   D4:      next = head.ptr->next    // Read Head.ptr->next
   D5:      if head == Q->Head	     // Are head, tail, and next consistent?
   D6:         if head.ptr == tail.ptr // Is queue empty or Tail falling behind?
   D7:            if next.ptr == NULL  // Is queue empty?
   D8:               return FALSE      // Queue is empty, couldn't dequeue
   D9:            endif
                  // Tail is falling behind.  Try to advance it
  D10:            CAS(&Q->Tail, tail, <next.ptr, tail.count+1>)
  D11:         else		     // No need to deal with Tail
                  // Read value before CAS
                  // Otherwise, another dequeue might free the next node
  D12:            *pvalue = next.ptr->value
                  // Try to swing Head to the next node
  D13:            if CAS(&Q->Head, head, <next.ptr, head.count+1>)
  D14:               break             // Dequeue is done.  Exit loop
  D15:            endif
  D16:         endif
  D17:      endif
  D18:   endloop
  D19:   free(head.ptr)		     // It is safe now to free the old node
  D20:   return TRUE                   // Queue was not empty, dequeue succeeded
```

## Two-Lock Concurrent Queue Algorithm

```
structure node_t {value: data type, next: pointer to node_t}
  structure queue_t {Head: pointer to node_t, Tail: pointer to node_t,
                        H_lock: lock type, T_lock: lock type}
  
  initialize(Q: pointer to queue_t)
     node = new_node()		// Allocate a free node
     node->next = NULL          // Make it the only node in the linked list
     Q->Head = Q->Tail = node	// Both Head and Tail point to it
     Q->H_lock = Q->T_lock = FREE	// Locks are initially free
  
  enqueue(Q: pointer to queue_t, value: data type)
     node = new_node()	        // Allocate a new node from the free list
     node->value = value		// Copy enqueued value into node
     node->next = NULL          // Set next pointer of node to NULL
     lock(&Q->T_lock)		// Acquire T_lock in order to access Tail
        Q->Tail->next = node	// Link node at the end of the linked list
        Q->Tail = node		// Swing Tail to node
     unlock(&Q->T_lock)		// Release T_lock
  
  dequeue(Q: pointer to queue_t, pvalue: pointer to data type): boolean
     lock(&Q->H_lock)	        // Acquire H_lock in order to access Head
        node = Q->Head		// Read Head
        new_head = node->next	// Read next pointer
        if new_head == NULL	// Is queue empty?
           unlock(&Q->H_lock)	// Release H_lock before return
           return FALSE		// Queue was empty
        endif
        *pvalue = new_head->value	// Queue not empty.  Read value before release
        Q->Head = new_head	// Swing Head to next node
     unlock(&Q->H_lock)		// Release H_lock
     free(node)			// Free node
     return} TRUE		// Queue was not empty, dequeue succeeded
```



# std::atomic

`std::atomic` 是用于在并发环境下对共享变量进行原子操作的工具。在多线程程序中，当多个线程同时访问和修改同一个变量时，可能会发生数据竞争（data race），导致未定义的行为发生。

通过使用 `std::atomic`，可以确保对共享变量的某些操作是原子的，也就是说它们作为一个不可分割的单个操作执行，不受其他线程的干扰。这保证了变量的访问或修改在并发环境中是同步的，避免了数据竞争。

`std::atomic` 模板类提供了一系列原子操作，例如加载（load）、存储（store）、交换（exchange）、比较并交换（compare-and-swap）和加法操作（fetch-and-add）。这些操作可以用于对变量进行原子的读取-修改-写入操作，确保修改在并发环境中以一致且安全的方式进行。

使用 `std::atomic` 可以编写线程安全的代码，它提供了原子性和同步的保证，而无需显式地使用锁或互斥量。它是并发编程中的重要工具，有助于保证多线程应用程序的正确性和可靠性。

```c++
template<typename T>
struct Node {
    T value;
    std::atomic<Node<T>*> next;
};
```

`std::atomic<Node<T>*> next;` 这样写的目的是声明一个名为 `next` 的原子指针变量，用于在多线程环境下对指针进行原子操作。

在并发环境中，当多个线程同时访问和修改同一个指针时，可能会出现数据竞争和不一致的问题。为了避免这种情况，我们可以使用 `std::atomic` 来确保对指针的操作是原子的。

`std::atomic` 提供了原子操作的特性，它可以确保指针的加载、存储和其他操作在并发情况下是原子的，即不会被其他线程中断或干扰。这样可以保证指针的操作在整个程序中是同步和一致的，避免了潜在的数据竞争问题。

在上述代码中，`std::atomic<Node<T>*> next;` 声明了一个原子指针变量 `next`，用于存储节点的下一个指针。通过使用原子指针，我们可以在多线程环境下安全地对链表进行操作，避免了多线程访问同一个节点指针时的竞争条件。这有助于确保链表操作的正确性和线程安全性。

## store

```c++
node -> next.store(nullptr); // store存储
```

`node->next.store(nullptr);` 这行代码将指针 `node` 的 `next` 成员变量存储为 `nullptr`。

在并发编程中，为了保证线程安全性，我们需要使用原子操作来修改共享变量。`std::atomic<T>` 提供了原子操作的功能，其中包括 `store()` 函数用于原子地存储一个新的值。

在这里，`node->next` 是一个原子指针，调用 `store(nullptr)` 将该指针存储为 `nullptr`，即将 `node` 的 `next` 成员设置为 `nullptr`。通过使用原子操作的 `store()`，我们确保了该操作的原子性，即在并发环境中不会被其他线程中断或干扰。

这样的操作通常用于在数据结构中修改指针的指向，以确保在多线程环境中的一致性和正确性。

# Non_Block

## Enqueue

```c++
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
                if (std::atomic_compare_exchange_weak(&tail -> next, &next, node)) {
                    // std::atomic对象引用 期望值 新值
                    // 函数会比较 std::atomic 对象的当前值与期望值，如果相等，
                    // 则将新值存储到 std::atomic 对象中，并返回 true；否则，不进行任何操作，并返回 false。
                    // weak 表示在失败时不进行重试，直接返回
                    break;
                }
            } else {
                std::atomic_compare_exchange_weak(&Q.tail, &tail, next);
            }
        }
    }
    Q.tail.compare_exchange_weak(Q.tail.load(), node);
}
```

这段代码实现了将元素入队（enqueue）的操作。

在函数内部，首先创建一个新的节点 `node`，并将传入的值 `value` 存储在该节点的 `value` 成员中。然后，使用原子操作 `store` 将 `node` 的 `next` 成员设置为 `nullptr`，确保在并发环境中的原子性。

接下来是一个无限循环，用于尝试将节点 `node` 加入队列。在循环中，首先获取当前的队尾节点 `tail` 和其下一个节点 `next`。然后进行条件判断：

- 如果当前的 `tail` 与之前获取的 `tail` 相等，表示判断仍然有效：
  - 如果 `next` 为 `nullptr`，即队尾节点是当前队列中的最后一个节点，则尝试使用原子操作 `compare_exchange_weak` 将 `node` 设置为队尾节点的下一个节点。如果操作成功，则说明入队成功，跳出循环。
  - 如果 `next` 不为 `nullptr`，表示队尾节点已经不是最后一个节点，可能有其他线程已经进行了入队操作，因此尝试使用原子操作 `compare_exchange_weak` 将队尾节点 `tail` 更新为其下一个节点 `next`，继续下一次循环。
- 如果当前的 `tail` 与之前获取的 `tail` 不相等，表示判断无效，可能有其他线程同时进行了入队操作，需要重新获取新的 `tail` 和 `next`。

最后，使用原子操作 `compare_exchange_weak` 将队尾节点 `tail` 更新为新入队的节点 `node`，确保更新成功。

这样，通过循环的尝试和原子操作的使用，实现了线程安全的元素入队操作。