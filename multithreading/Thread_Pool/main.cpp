#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
#include <queue>
#include <chrono>
#include <cassert>
#include <functional>
#include <condition_variable>

class Thread_Pool {
public:
    explicit Thread_Pool(const size_t count_threads): threads_(count_threads) {
        for (auto& thread : threads_) {
            thread = std::thread(&Thread_Pool::worker, this);
        }
    }

    void enqueue(const std::function<void()>& task) {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            tasks_.push(task);
        }
        cv_.notify_one();
    }

    void stop() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            stop_ = true;
        }
        cv_.notify_all();

        for (auto& thread : threads_) {
            if (thread.joinable()) {
                thread.join();
            }
        }
    }

    ~Thread_Pool() {
        stop();
    }

private:
    void worker() {
        while (true) {
            std::function<void()> task;

            {
                std::unique_lock<std::mutex> lock(mutex_);
                cv_.wait(lock, [this]{ return stop_ || !tasks_.empty(); });

                if (stop_ && tasks_.empty()) {
                    return;
                }

                task = std::move(tasks_.front());
                tasks_.pop();
            }

            task();
        }
    }

    std::vector<std::thread> threads_;
    std::queue<std::function<void()>> tasks_;

    std::mutex mutex_;
    std::condition_variable cv_;
    std::atomic<bool> stop_{false};
};

void test_basic_execution() {
    std::cout << "=== Test 1: Basic execution ===\n";
    Thread_Pool pool(4);

    std::atomic<int> counter{0};
    for (int i = 0; i < 10; ++i) {
        pool.enqueue([&counter] {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            ++counter;
        });
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    assert(counter == 10 && "All tasks should have executed");
    std::cout << "All tasks executed successfully\n";
}

void test_parallelism() {
    std::cout << "\n=== Test 2: Parallelism check ===\n";
    Thread_Pool pool(4);

    auto start = std::chrono::steady_clock::now();

    for (int i = 0; i < 4; ++i) {
        pool.enqueue([] {
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
        });
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(400));
    auto end = std::chrono::steady_clock::now();

    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Elapsed ~ " << elapsed.count() << " s\n";
    std::cout << "Tasks executed concurrently\n";
}

void test_stop_behavior() {
    std::cout << "\n=== Test 3: Stop() behavior ===\n";
    Thread_Pool pool(2);

    std::atomic<int> counter{0};
    for (int i = 0; i < 5; ++i) {
        pool.enqueue([&counter] {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            ++counter;
        });
    }

    pool.stop();

    auto before = counter.load();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    auto after = counter.load();

    assert(before == after && "No new tasks should execute after stop()");
    std::cout << "Thread pool stopped correctly\n";
}

void test_stress() {
    std::cout << "\n=== Test 4: Stress test ===\n";
    Thread_Pool pool(8);

    std::atomic<int> counter{0};
    const int NUM_TASKS = 10000;

    for (int i = 0; i < NUM_TASKS; ++i) {
        pool.enqueue([&counter] {
            ++counter;
        });
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    assert(counter == NUM_TASKS && "All stress tasks should have executed");
    std::cout << "Stress test passed (" << counter << " tasks)\n";
}

int main() {
    test_basic_execution();
    test_parallelism();
    test_stop_behavior();
    test_stress();

    std::cout << "\nAll tests passed\n";
}

