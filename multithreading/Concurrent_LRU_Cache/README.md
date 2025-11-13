# Concurrent LRU Cache

Thread-safe LRU (Least Recently Used) cache implementation with striped locking for high concurrency.

## Architecture

The `Concurrent_LRU` class implements a thread-safe LRU cache using striped locking:
- Data is partitioned into 16 stripes (buckets) based on key hash
- Each stripe has its own `std::shared_mutex` for fine-grained locking
- LRU eviction policy: least recently used items are removed when capacity is exceeded
- Read operations use shared locks, write operations use exclusive locks
- Per-segment capacity limit for each stripe

## Components

- `Concurrent_LRU<Key, Value>` - main cache class
  - `insert(key, value)` - adds or updates key-value pair, evicts LRU if needed
  - `get(key)` - retrieves value by key, updates LRU order, returns `std::optional<Value>`
  - `size()` - returns total number of elements across all stripes
  - `empty()` - checks if cache is empty

## Data Structures

- `std::list<std::pair<Key, Value>>` - maintains LRU order (front = most recent)
- `std::unordered_map<Key, iterator>` - O(1) key lookup to list iterator
- `std::array<Striped, 16>` - 16 stripes for concurrent access

## Synchronization

- `std::shared_mutex` per stripe for read-write lock semantics
- Hash-based stripe selection: `hash(key) % 16`
- Read operations use `std::shared_lock`
- Write operations use `std::unique_lock`
