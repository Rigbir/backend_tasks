# Striped Unordered Map

Thread-safe hash map implementation with striped locking for concurrent read-write access.

## Architecture

The `Striped_UM` class implements a thread-safe hash map using striped locking:
- Data is partitioned into 16 stripes (buckets) based on key hash
- Each stripe has its own `std::shared_mutex` for fine-grained locking
- Read operations use shared locks for concurrent reads
- Write operations use exclusive locks for thread-safe modifications
- Hash-based distribution reduces lock contention

## Components

- `Striped_UM<T>` - main map class
  - `get(T key)` - retrieves value by key, returns `std::optional<T>`
  - `insert(T key, T value)` - adds or updates key-value pair
  - `erase(T key)` - removes key-value pair

## Data Structures

- `std::unordered_map<T, T>` per stripe for key-value storage
- `std::array<Striped, 16>` - 16 stripes for concurrent access
- `std::shared_mutex` per stripe for read-write lock semantics

## Synchronization

- `std::shared_mutex` per stripe for read-write lock semantics
- Hash-based stripe selection: `hash(key) % 16`
- Read operations use `std::shared_lock`
- Write operations use `std::unique_lock`
