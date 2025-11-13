# Thread Pool

Thread pool implementation for parallel task execution with graceful shutdown.

## Architecture

The `Thread_Pool` class manages a fixed-size pool of worker threads:
- Fixed number of worker threads created at construction
- Task queue for pending work items
- Worker threads continuously process tasks from queue
- Graceful shutdown: stops accepting new tasks and waits for completion
- RAII pattern: automatic cleanup in destructor

## Components

- `Thread_Pool` - main thread pool class
  - `enqueue(function<void()>)` - adds task to queue for execution
  - `stop()` - signals workers to stop and waits for thread completion
  - `~Thread_Pool()` - automatically stops pool on destruction
  - `worker()` - worker thread function that processes tasks

## Synchronization

- `std::mutex` protects task queue
- `std::condition_variable` for waiting on new tasks
- `std::atomic<bool>` for thread-safe stop flag
- `std::queue<std::function<void()>>` for task storage

## Task Execution

- Tasks are executed asynchronously by worker threads
- Tasks are processed in FIFO order
- Multiple tasks can run concurrently (up to thread count)
