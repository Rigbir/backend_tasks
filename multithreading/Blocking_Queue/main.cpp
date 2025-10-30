#include <iostream>
#include <thread>
#include <atomic>
#include <mutex>
#include <queue>
#include <condition_variable>

template <typename T>
class BlockingQueue {
public:
    explicit BlockingQueue(const size_t capacity): capacity_(capacity), running_(true) {}

    void push(const T& value) {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_not_full.wait(lock, [this]() {
           return data_queue.size() < capacity_ || !running_;
        });

        if (!running_) return;

        data_queue.push(value);
        cv_not_empty.notify_one();
    }

    std::optional<T> pop_for(std::chrono::milliseconds timeout) {
         return pop_impl(false, timeout);
    }

    T pop() {
        auto val = pop_impl(true);
        if (!val.has_value()) throw std::runtime_error("Queue stopped");
        return *val;
    }

    std::optional<T> pop_impl(bool wait_forever, std::chrono::milliseconds timeout = {}) {
        std::unique_lock<std::mutex> lock(mutex_);

        bool ready = false;
        if (wait_forever) {
            cv_not_empty.wait(lock, [this]() { return !data_queue.empty() || !running_; });
            ready = !data_queue.empty();
        } else {
            ready = cv_not_empty.wait_for(lock, timeout, [this]() { return !data_queue.empty() || !running_; });
        }

        if (!ready) {
            return std::nullopt;
        }

        if (!running_ || data_queue.empty()) {
            return std::nullopt;
        }

        T value = data_queue.front();
        data_queue.pop();
        cv_not_full.notify_one();

        return value;
    }

    void stop() {
        running_ = false;
        cv_not_empty.notify_all();
        cv_not_full.notify_all();
    }

    ~BlockingQueue() {
        stop();
    }

private:
    size_t capacity_;
    std::queue<T> data_queue;

    std::mutex mutex_;                      // protects the data_queue
    std::condition_variable cv_not_empty;   // signals that the queue is not empty
    std::condition_variable cv_not_full;    // signals that the queue is not full

    std::atomic<bool> running_;
};

void test_basic() {
    BlockingQueue<int> bq(10);

    std::vector<std::thread> producers;
    for (int i = 0; i < 3; ++i) {
        producers.emplace_back([&, i]() {
            for (int j = 0; j < 100; ++j) {
                bq.push(i * 1000 + j);
                std::cout << "[Producer] pushing " << i * 1000 + j << '\n';
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
            }
        });
    }

    std::vector<std::thread> consumers;
    for (int i = 0; i < 2; ++i) {
        consumers.emplace_back([&]() {
            for (int j = 0; j < 150; ++j) {
                int value = bq.pop();
                std::cout << "   [Consumer] got " << value << '\n';
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        });
    }

    for (auto& p : producers) p.join();
    for (auto& c : consumers) c.join();
}

void test_with_timeout() {
    BlockingQueue<int> bq(10);

    std::thread producer([&]() {
        for (int i = 0; i < 90; ++i) {
            bq.push(i * 1000);
            std::cout << "[Producer] pushing " << i * 1000 << '\n';
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }

        bq.stop();
    });

    std::vector<std::thread> consumers;
    for (int i = 0; i < 3; ++i) {
        consumers.emplace_back([&]() {
            for (int j = 0; j < 40; ++j) {
                std::optional<int> value = bq.pop_for(std::chrono::milliseconds(10));

                if (value.has_value()) {
                    std::cout << "   [Consumer] got " << *value << '\n';
                } else {
                    std::cout << "   [Consumer] timeout\n";
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        });
    }

    producer.join();
    for (auto& c : consumers) c.join();
}

int main() {
    test_basic();
    test_with_timeout();
}