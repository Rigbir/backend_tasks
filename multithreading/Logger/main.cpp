#include <iostream>
#include <thread>
#include <atomic>
#include <mutex>

class Logger {
public:
    Logger(): running_(false) {}

    void start() {
        running_ = true;
        for (int i = 1; i <= 5; ++i) {
            threads_.emplace_back([i, this]() {
               for (int j = 1; j <= 5; ++j) {
                   std::lock_guard<std::mutex> lock(mutex_);
                   std::cout << "Threads " << i << ": msg: " << j << '\n';
               }
            });
        }
    }

    void stop() {
        running_ = false;
        for (auto& thread : threads_) {
            if (thread.joinable()) {
                thread.join();
            }
        }
    }

    ~Logger() {
        stop();
    }

private:
    std::vector<std::thread> threads_;
    std::atomic<bool> running_;
    std::mutex mutex_;
};

int main() {
    Logger logger;
    logger.start();
    logger.stop();
}
