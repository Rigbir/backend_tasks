# Blocking Queue

Thread-safe blocking queue with bounded capacity for producer-consumer scenarios.

## Architecture

The `BlockingQueue` class implements a thread-safe queue with blocking operations:
- Bounded capacity to prevent unbounded memory growth
- Blocking `push()` operation waits when queue is full
- Blocking `pop()` operation waits when queue is empty
- Support for graceful shutdown via `stop()` method
- Optional timeout-based `pop_for()` for non-blocking retrieval

## Components

- `BlockingQueue<T>` - main queue class
  - `push(const T&)` - adds element to queue, blocks if full
  - `pop()` - removes and returns element, blocks if empty, throws if stopped
  - `pop_for(timeout)` - removes element with timeout, returns `std::optional<T>`
  - `stop()` - signals all waiting threads to stop
  - `~BlockingQueue()` - automatically stops queue on destruction

## Synchronization

- `std::mutex` protects internal queue state
- `std::condition_variable` for waiting on not-empty condition
- `std::condition_variable` for waiting on not-full condition
- `std::atomic<bool>` for thread-safe stop flag
