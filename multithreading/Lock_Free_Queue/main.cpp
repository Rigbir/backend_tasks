#include <iostream>
#include <thread>
#include <atomic>
#include <mutex>

template <typename T>
class Lock_Free_Queue {
public:
    Lock_Free_Queue() {
        Node* dummy = new Node{};
        head.store(dummy);
        tail.store(dummy);
    }

    void push(T value) {
        Node* new_Node = new Node{value, nullptr};
        Node* old_tail;

        while (true) {
            old_tail = tail.load();
            Node* next = old_tail->next.load();

            if (!next) {
                if (old_tail->next.compare_exchange_weak(next, new_Node)) {
                    break;
                }
            } else {
                tail.compare_exchange_weak(old_tail, next);
            }
        }

        tail.compare_exchange_weak(old_tail, new_Node);
    }

    bool pop(T& result) {
        while (true) {
            Node* old_head = head.load();
            Node* next = old_head->next.load();

            if (!next) return false;

            if (head.compare_exchange_weak(old_head, next)) {
                result = next->value;
                delete old_head;
                return true;
            }
        }
    }

private:
    struct Node {
        T value;
        std::atomic<Node*> next{nullptr};
    };

    std::atomic<Node*> head;
    std::atomic<Node*> tail;
};

int main() {
    Lock_Free_Queue<int> q;
    std::mutex cout_mutex;

    std::vector<std::thread> producers;
    for (int i = 0; i < 4; ++i) {
        producers.emplace_back([&q, i, &cout_mutex]() mutable {
            for (int j = 0; j < 10; ++j) {
                q.push(i * 100 + j);
                {
                    std::lock_guard<std::mutex> lock(cout_mutex);
                    std::cout << "[Producer " << i << "] push: " << i * 100 + j << "\n";
                }
            }
        });
    }

    std::vector<std::thread> consumers;
    for (int i = 0; i < 2; ++i) {
        consumers.emplace_back([&q, i, &cout_mutex]() mutable {
            int value;
            for (int j = 0; j < 20; ++j) {
                if (q.pop(value)) {
                    std::lock_guard<std::mutex> lock(cout_mutex);
                    std::cout << "   [Consumer " << i << "] got: " << value << "\n";
                }
            }
        });
    }

    for (auto& p : producers) p.join();
    for (auto& c : consumers) c.join();
}