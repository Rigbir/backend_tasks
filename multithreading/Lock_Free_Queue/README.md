# Lock-Free Queue

Lock-free queue implementation using atomic operations for high-performance concurrent access.

## Architecture

The `Lock_Free_Queue` class implements a lock-free queue using atomic compare-and-swap operations:
- Single-linked list structure with dummy head node
- `push()` operation uses atomic CAS to append to tail
- `pop()` operation uses atomic CAS to remove from head
- No mutexes or blocking operations - suitable for high-contention scenarios
- Memory management: nodes are allocated on push and deallocated on pop

## Components

- `Lock_Free_Queue<T>` - main queue class
  - `push(T)` - adds element to queue (lock-free)
  - `pop(T&)` - removes element from queue, returns `bool` indicating success (lock-free)

## Data Structures

- `Node` structure with `T value` and `std::atomic<Node*> next`
- `std::atomic<Node*>` for head pointer
- `std::atomic<Node*>` for tail pointer

## Lock-Free Algorithm

- `push()`: CAS loop to atomically update tail->next, then update tail
- `pop()`: CAS loop to atomically update head, returns value from next node
- Helps with tail pointer advancement if another thread already updated it
