#include <iostream>
#include <thread>
#include <mutex>
#include <array>
#include <shared_mutex>
#include <unordered_map>

template <typename T>
class Striped_UM {
public:
    std::optional<T> get(T key) {
        auto& stripe = stripes_[get_striped(key)];
        std::shared_lock<std::shared_mutex> lock(stripe.shm_);

        if (stripe.data_.contains(key)) {
            return stripe.data_[key];
        }

        return std::nullopt;
    }

    void insert(T key, T value) {
        auto& stripe = stripes_[get_striped(key)];
        std::unique_lock<std::shared_mutex> lock(stripe.shm_);
        stripe.data_[key] = value;
    }

    void erase(T key) {
        auto& stripe = stripes_[get_striped(key)];
        std::unique_lock<std::shared_mutex> lock(stripe.shm_);
        stripe.data_.erase(key);
    }
    
private:
    static const size_t N = 16;

    struct Striped {
        std::unordered_map<T, T> data_;
        std::shared_mutex shm_;
    };

    std::array<Striped, N> stripes_;
    
    size_t get_striped(const T& key) {
        return std::hash<T>{}(key) % N;
    }
};

void stress_insert_get(Striped_UM<int>& map, int num_threads, int num_keys) {
    std::vector<std::thread> writers;
    for (int t = 0; t < num_threads; ++t) {
        writers.emplace_back([&map, t, num_keys]() {
            for (int i = 0; i < num_keys; ++i) {
                int key = i + t * num_keys;
                map.insert(key, key*10);
            }
        });
    }

    std::vector<std::thread> readers;
    for (int t = 0; t < num_threads; ++t) {
        readers.emplace_back([&map, num_keys]() {
            for (int i = 0; i < num_keys*8; ++i) {
                map.get(i);
            }
        });
    }

    for (auto& th : writers) th.join();
    for (auto& th : readers) th.join();

    std::cout << "Stress insert/get finished\n";
}

void stress_erase_get(Striped_UM<int>& map, int num_threads, int num_keys) {
    std::vector<std::thread> erasers;
    for (int t = 0; t < num_threads; ++t) {
        erasers.emplace_back([&map, t, num_keys]() {
            for (int i = 0; i < num_keys; ++i) {
                int key = i + t * num_keys;
                map.erase(key);
            }
        });
    }

    std::vector<std::thread> readers;
    for (int t = 0; t < num_threads; ++t) {
        readers.emplace_back([&map, num_keys]() {
            for (int i = 0; i < num_keys*8; ++i) {
                map.get(i);
            }
        });
    }

    for (auto& th : erasers) th.join();
    for (auto& th : readers) th.join();

    std::cout << "Stress erase/get finished\n";
}

int main() {
    Striped_UM<int> map;

    const int NUM_THREADS = 8;
    const int NUM_KEYS = 1000;

    stress_insert_get(map, NUM_THREADS, NUM_KEYS);
    stress_erase_get(map, NUM_THREADS, NUM_KEYS);

    int count = 0;
    for (int i = 0; i < NUM_THREADS * NUM_KEYS; ++i) {
        if (map.get(i)) ++count;
    }

    std::cout << "Final element count: " << count << std::endl;
}