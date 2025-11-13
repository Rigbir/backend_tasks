# Logger

Thread-safe logger with mutex synchronization for concurrent message output.

## Architecture

The `Logger` class provides thread-safe logging functionality:
- Multiple worker threads can log messages concurrently
- `std::mutex` protects output stream from race conditions
- Thread lifecycle management: start/stop methods
- RAII pattern: automatic cleanup in destructor

## Components

- `Logger` - main logger class
  - `start()` - creates and starts worker threads
  - `stop()` - signals threads to stop and waits for completion
  - `~Logger()` - automatically stops logger on destruction

## Synchronization

- `std::mutex` protects `std::cout` output
- `std::lock_guard` for automatic mutex locking
- `std::atomic<bool>` for thread-safe running flag
