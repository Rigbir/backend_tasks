#include <iostream>
#include <thread>
#include <vector>
#include <optional>
#include <shared_mutex>
#include <unordered_map>
#include <list>
#include <array>
#include <functional>

template <typename Key, typename Value>
class Concurrent_LRU {
public:
    explicit Concurrent_LRU(size_t per_segment_capacity): capacity_(per_segment_capacity) {}

    void insert(const Key& key, const Value& value) noexcept {
        auto& stripe = striped_[get_striped(key)];
        std::unique_lock<std::shared_mutex> lock(stripe.shm_);

        if (stripe.data_.contains(key)) {
            stripe.list_.erase(stripe.data_[key]);
        }

        stripe.list_.emplace_front(key, value);
        stripe.data_[key] = stripe.list_.begin();

        if (stripe.list_.size() > capacity_) {
            auto& key_to_remove = stripe.list_.back();
            stripe.data_.erase(key_to_remove.first);
            stripe.list_.pop_back();
        }
    }

    std::optional<Value> get(const Key& key) noexcept {
        auto& stripe = striped_[get_striped(key)];
        std::unique_lock<std::shared_mutex> lock(stripe.shm_);

        if (!stripe.data_.contains(key)) {
            return std::nullopt;
        }

        auto it = stripe.data_.find(key);
        if (it == stripe.data_.end()) {
            return std::nullopt;
        }

        stripe.list_.splice(stripe.list_.begin(), stripe.list_, it->second);
        return it->second->second;
    }

    size_t size() const noexcept {
        size_t total = 0;
        for (const auto& stripe : striped_) {
            std::shared_lock<std::shared_mutex> lock(stripe.shm_);
            total += stripe.list_.size();
        }
        return total;
    }

    bool empty() const noexcept {
        for (const auto& stripe : striped_) {
            std::shared_lock<std::shared_mutex> lock(stripe.shm_);
            if (!stripe.list_.empty()) return false;
        }
        return true;
    }

private:
    const size_t capacity_;
    static const size_t N = 16;

    struct Striped {
        std::unordered_map<Key, typename std::list<std::pair<Key, Value>>::iterator> data_;
        std::list<std::pair<Key, Value>> list_;
        mutable std::shared_mutex shm_;
    };
    std::array<Striped, N> striped_;

    size_t get_striped(const Key& key) const {
        return std::hash<Key>{}(key) % N;
    }
};

void stress_insert_get(Concurrent_LRU<int,int>& cache, int num_threads, int num_keys) {
    std::vector<std::thread> threads;

    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([&cache, t, num_keys]() {
            for (int i = 0; i < num_keys; ++i) {
                int key = i + t*num_keys;
                cache.insert(key, key*10);
                auto val = cache.get(key);
                if (!val || *val != key*10) {
                    std::cout << "Wrong insert/get key=" << key << std::endl;
                }
            }
        });
    }

    for (auto& th : threads) th.join();
    std::cout << "Stress insert/get finished\n";
}

void stress_lru_eviction(Concurrent_LRU<int,int>& cache, int num_threads, int num_keys) {
    std::vector<std::thread> threads;

    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([&cache, t, num_keys, num_threads]() {
            for (int i = 0; i < num_keys; ++i) {
                int key = i + t*num_keys;
                cache.insert(key, key*10);

                int read_key = rand() % (num_threads*num_keys);
                cache.get(read_key);
            }
        });
    }

    for (auto& th : threads) th.join();
    std::cout << "Stress LRU eviction finished\n";

    std::cout << "Cache size after eviction: " << cache.size() << std::endl;
}

int main() {
    const int NUM_THREADS = 8;
    const int NUM_KEYS = 100;

    const size_t per_segment_capacity = 100;
    Concurrent_LRU<int,int> cache(per_segment_capacity);

    stress_insert_get(cache, NUM_THREADS, NUM_KEYS);
    stress_lru_eviction(cache, NUM_THREADS, NUM_KEYS);
}